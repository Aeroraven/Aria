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
					{ model[k].texCoords[i * 2],model[k].texCoords[i * 2 + 1], k * 1.0f,1.00 });
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
	AnthemVertexBuffer* AnthemSimpleModelIntegrator::getVertexBuffer() {
		return this->vx;
	}
	AnthemIndexBuffer* AnthemSimpleModelIntegrator::getIndexBuffer() {
		return this->ix;
	}
	AnthemIndirectDrawBuffer* AnthemSimpleModelIntegrator::getIndirectBuffer() {
		return this->indirect;
	}
}