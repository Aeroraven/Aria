#include "../../../include/components/utility/AnthemSimpleModelIntegrator.h"

namespace Anthem::Components::Utility {
	bool AnthemSimpleModelIntegrator::loadModel(AnthemSimpleToyRenderer* renderer, std::vector<AnthemUtlSimpleModelStruct> model, uint32_t cpuJobs) {
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
	bool AnthemSimpleModelIntegrator::loadModelRayTracing(AnthemSimpleToyRenderer* renderer, const std::vector<AnthemUtlSimpleModelStruct>& model) {
		auto& rd = renderer;
		std::vector<uint32_t> numVerts;
		uint32_t totalIndices = 0;
		uint32_t totalVerts = 0;
		for (auto i : AT_RANGE2(model.size())) {
			numVerts.push_back(model[i].positions.size() / 3);
			totalIndices += model[i].indices.size();
			totalVerts += model[i].positions.size() / 3;
		}

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
				ANTH_LOGI("OB->", accu, ",", accuIdx);
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

		// Tex Buffer
		rd->createDescriptorPool(&descTex);
		using texBufferSType = std::remove_cv_t<decltype(texBuffer)>;
		std::function<void(texBufferSType)> texBufferPrepare = [&](texBufferSType w)->void {
			uint32_t ci = 0;
			for (auto x : model)
				for (int i = 0; i < x.texCoords.size(); i += 2) {
					w->setInput(ci++, { x.texCoords[i + 0],x.texCoords[i + 1] });
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
		return ret;
	}
}