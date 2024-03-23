#include "../../../include/components/utility/AnthemSimpleModelIntegrator.h"
#include "../../../include/core/math/AnthemLinAlg.h"
#include "../../../include/core/math/AnthemMathAbbrs.h"

namespace Anthem::Components::Utility {
	bool AnthemSimpleModelIntegrator::loadModel(AnthemSimpleToyRenderer* renderer, std::vector<AnthemUtlSimpleModelStruct> model, uint32_t cpuJobs) {
		using namespace Anthem::Core::Math::Abbr;
		using namespace Anthem::Core::Math;

		auto& rd = renderer;
		rd->createVertexBuffer(&vx);
		rd->createIndexBuffer(&ix);
		rd->createIndirectDrawBuffer(&indirect);

		std::vector<uint32_t> prs;
		uint32_t prx = 0;
		for (auto& p : model) {
			prs.push_back(prx);
			prx += p.numVertices;
		}
		vx->setTotalVertices(prx);
		auto childJob = [&](int k)->void {
			for (int i = 0; i < model[k].numVertices; i++) {
				vx->insertData(i + prs[k],
					{ model[k].positions[i * 3],model[k].positions[i * 3 + 1],model[k].positions[i * 3 + 2],1.0 },
					{ model[k].normals[i * 3],model[k].normals[i * 3 + 1],model[k].normals[i * 3 + 2],0.0 },
					{ model[k].texCoords[i * 2],model[k].texCoords[i * 2 + 1], k * 1.0f + 0.5f,1.00 });
			}

		};
		std::vector<std::thread> th;
		for (int k = 0; k < prs.size(); k++) {
			std::thread childExec(childJob, k);
			th.push_back(std::move(childExec));
		}
		for (int k = 0; k < prs.size(); k++) {
			th[k].join();
		}
		std::vector<uint32_t> ixList;
		uint32_t alcx = 0;
		for (int k = 0; k < prs.size(); k++) {
			ixList.insert(ixList.end(), model[k].indices.begin(), model[k].indices.end());
			indirect->addIndirectDrawCommand(1, 0, model[k].indices.size(), alcx, prs[k]);
			alcx += model[k].indices.size();
		}
		ix->setIndices(ixList);
		return true;
	}
	float AnthemSimpleModelIntegrator::getLightAreas() {
		return this->totalLightAreas;
	}
	uint32_t AnthemSimpleModelIntegrator::getLightFaces() {
		return this->totalLightFaces;
	}
	bool AnthemSimpleModelIntegrator::loadModelRayTracing(AnthemSimpleToyRenderer* renderer, const std::vector<AnthemUtlSimpleModelStruct>& model,
		const std::set<int> lightIds) {
		using namespace Anthem::Core::Math::Abbr;
		using namespace Anthem::Core::Math;


		auto& rd = renderer;
		std::vector<uint32_t> numVerts;
		uint32_t totalIndices = 0;
		uint32_t totalVerts = 0;

		uint32_t totalLightInds = 0;
		for (auto i : AT_RANGE2(model.size())) {
			numVerts.push_back(model[i].positions.size() / 3);
			totalIndices += model[i].indices.size();
			totalVerts += model[i].positions.size() / 3;
			if (lightIds.count(i)) {
				totalLightInds += model[i].indices.size();
			}
		}
		ANTH_LOGI("Total Indices:", totalIndices);
		ANTH_LOGI("Total Triangles:", totalIndices / 3);
		ANTH_LOGI("Total Lighting Triangles:", totalLightInds / 3);
		ANTH_LOGI("Total Instances:", model.size());

		// Offset Buffer
		rd->createDescriptorPool(&descOffset);
		using offsetBufferSType = std::remove_cv_t<decltype(offsetBuffer)>;
		std::function<void(offsetBufferSType)> offsetBufferPrepare = [&](offsetBufferSType w)->void {
			uint32_t accu = 0;
			uint32_t accuIdx = 0;
			for (auto i : AT_RANGE2(numVerts.size())) {
				w->setInput(i, { accu });
				w->setInput(i + numVerts.size(), { accuIdx });
				
				accu += numVerts[i];
				accuIdx += model[i].indices.size();

			}
		};
		rd->createShaderStorageBuffer(&offsetBuffer, numVerts.size() * 2, 0, descOffset, std::make_optional(offsetBufferPrepare), -1);

		// Index Buffer
		rd->createDescriptorPool(&descIndex);
		using indexBufferSType = std::remove_cv_t<decltype(indexBuffer)>;
		std::function<void(indexBufferSType)> indexBufferPrepare = [&](indexBufferSType w)->void {
			uint32_t ci = 0;
			for (auto x : model) 
				for (auto i : x.indices)
					w->setInput(ci++, { i });
		};
		rd->createShaderStorageBuffer(&indexBuffer, totalIndices, 0, descIndex, std::make_optional(indexBufferPrepare), -1);

		// Light Index Bufer
		rd->createDescriptorPool(&descLightIdx);
		using idLightBufSType = std::remove_cv_t<decltype(lightIndexBuffer)>;
		std::function<void(idLightBufSType)> indLightBufferPrepare= [&](idLightBufSType w)->void {
			uint32_t ci = 0;
			for (auto i : AT_RANGE2(model.size())) {
				if (lightIds.count(i) == 0)continue;
				for (int j = 0; j < model[i].indices.size(); j += 3) {
					w->setInput(ci++, { model[i].indices[j] });
					w->setInput(ci++, { model[i].indices[j+1] });
					w->setInput(ci++, { model[i].indices[j+2] });
					w->setInput(ci++, { static_cast<uint32_t>(i) });

					AtVecf3 v0 = { model[i].positions[model[i].indices[j] * 3],
						model[i].positions[model[i].indices[j] * 3 + 1] ,model[i].positions[model[i].indices[j] * 3 + 2] };
					AtVecf3 v1 = { model[i].positions[model[i].indices[j + 1] * 3],
						model[i].positions[model[i].indices[j + 1] * 3 + 1] ,model[i].positions[model[i].indices[j + 1] * 3 + 2] };
					AtVecf3 v2 = { model[i].positions[model[i].indices[j + 2] * 3],
						model[i].positions[model[i].indices[j + 2] * 3 + 1] ,model[i].positions[model[i].indices[j + 2] * 3 + 2] };

					AtVecf3 a1 = v1 - v0;
					AtVecf3 a2 = v2 - v0;
					AtVecf3 cr = AnthemLinAlg::cross3(a1, a2);
					totalLightAreas += cr.len() / 2;
					totalLightFaces++;
				}
			}
			ANTH_LOGI("Total Light Faces:", totalLightFaces);
		};
		rd->createShaderStorageBuffer(&lightIndexBuffer, totalLightInds/3*4, 0, descLightIdx, std::make_optional(indLightBufferPrepare), -1);

		// Pos Buffer
		rd->createDescriptorPool(&descPos);
		using posBufferSType = std::remove_cv_t<decltype(posBuffer)>;
		std::function<void(posBufferSType)> posBufferPrepare = [&](posBufferSType w)->void {
			uint32_t ci = 0;
			for(auto x:model)
				for (int i = 0; i < x.positions.size(); i+=3) {
					w->setInput(ci++, { x.positions[i + 0],x.positions[i + 1],x.positions[i + 2],1.0 });
				}
		};
		rd->createShaderStorageBuffer(&posBuffer, totalVerts, 0, descPos, std::make_optional(posBufferPrepare), -1);

		// Normal Buffer
		rd->createDescriptorPool(&descNormal);
		using normalBufferSType = std::remove_cv_t<decltype(normalBuffer)>;
		std::function<void(normalBufferSType)> normalBufferPrepare = [&](normalBufferSType w)->void {
			uint32_t ci = 0;
			for (auto x : model)
				for (int i = 0; i < x.normals.size(); i += 3) {
					w->setInput(ci++, { x.normals[i + 0],x.normals[i + 1],x.normals[i + 2],1.0 });
				}
		};
		rd->createShaderStorageBuffer(&normalBuffer, totalVerts, 0, descNormal, std::make_optional(normalBufferPrepare), -1);

		// Color Buffer
		rd->createDescriptorPool(&descColor);
		using colorBufferSType = std::remove_cv_t<decltype(colorBuffer)>;
		std::function<void(colorBufferSType)> colorBufferPrepare = [&](colorBufferSType w)->void {
			uint32_t ci = 0;
			for (auto x : model)
				for (int i = 0; i < x.positions.size(); i += 3) {
					w->setInput(ci++, { 
						static_cast<float>(x.pbrBaseColorFactor[0]),
						static_cast<float>(x.pbrBaseColorFactor[1]),
						static_cast<float>(x.pbrBaseColorFactor[2]),
						1.0 });
					//ANTH_LOGI(static_cast<float>(x.pbrBaseColorFactor[i + 0])," ", static_cast<float>(x.pbrBaseColorFactor[i + 1]));
				}
			};
		rd->createShaderStorageBuffer(&colorBuffer, totalVerts, 0, descColor, std::make_optional(colorBufferPrepare), -1);


		// Tex Buffer
		rd->createDescriptorPool(&descTex);
		using texBufferSType = std::remove_cv_t<decltype(texBuffer)>;
		std::function<void(texBufferSType)> texBufferPrepare = [&](texBufferSType w)->void {
			uint32_t ci = 0;
			for (auto x : model) {
				for (int i = 0; i < x.texCoords.size(); i += 2) {
					w->setInput(ci++, { x.texCoords[i + 0],x.texCoords[i + 1] });
				}
				if (x.texCoords.size() == 0) {
					for (int i = 0; i < x.normals.size(); i += 3) {
						w->setInput(ci++, { 0,0 });
					}
				}
			}
		};
		rd->createShaderStorageBuffer(&texBuffer, totalVerts, 0, descTex, std::make_optional(texBufferPrepare), -1);


		// Geometries
		asGeo.reserve(numVerts.size());
		asGeo.resize(numVerts.size());
		std::vector<float> transform = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
		};
		std::vector<std::vector<float>> transList;
		for (auto i : AT_RANGE2(numVerts.size())) {
			rd->createRayTracingGeometry(&asGeo[i], 3, model[i].positions, model[i].indices, transform);
		}

		// BLAS
		blasObj.reserve(numVerts.size());
		blasObj.resize(numVerts.size());
		for (auto i : AT_RANGE2(numVerts.size())) {
			rd->createBottomLevelAS(&blasObj[i]);
			blasObj[i]->addGeometry({ asGeo[i] });
			transList.push_back(transform);
			blasObj[i]->buildBLAS();
		}

		// Instances
		rd->createRayTracingInstance(&asInst, blasObj, transList);

		// TLAS
		rd->createTopLevelAS(&tlasObj);
		tlasObj->addInstance({ asInst });
		tlasObj->buildTLAS();
		return true;
	}
	AnthemVertexBuffer* AnthemSimpleModelIntegrator::getVertexBuffer() {
		return this->vx;
	}
	AnthemIndexBuffer* AnthemSimpleModelIntegrator::getIndexBuffer() {
		return this->ix;
	}
	AnthemIndirectDrawBuffer* AnthemSimpleModelIntegrator::getIndirectBuffer() {
		return this->indirect;
	}
	AnthemSimpleModelIntegratorRayTracingStructs AnthemSimpleModelIntegrator::getRayTracingParsedResult() {
		AnthemSimpleModelIntegratorRayTracingStructs ret;
		ret.asGeo = asGeo;
		ret.asInst = asInst;
		ret.blasObj = blasObj;
		ret.descIndex = descIndex;
		ret.descOffset = descOffset;
		ret.descNormal = descNormal;
		ret.descPos = descPos;
		ret.descTex = descTex;
		ret.indexBuffer = indexBuffer;
		ret.normalBuffer = normalBuffer;
		ret.offsetBuffer = offsetBuffer;
		ret.posBuffer = posBuffer;
		ret.texBuffer = texBuffer;
		ret.tlasObj = tlasObj;

		ret.descLightIdx = descLightIdx;
		ret.lightIndexBuffer = lightIndexBuffer;

		ret.descColor = descColor;
		ret.colorBuffer = colorBuffer;
		return ret;
	}
}