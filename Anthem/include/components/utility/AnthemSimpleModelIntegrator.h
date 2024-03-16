#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/utils/AnthemUtlSimpleModelStruct.h"
#include "../../core/renderer/AnthemSimpleToyRenderer.h"


namespace Anthem::Components::Utility {
	using namespace Anthem::Core;
	class AnthemSimpleModelIntegrator {
	private:
		AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>, AtAttributeVecf<4>>* vx = nullptr;
		AnthemIndexBuffer* ix = nullptr;
		AnthemIndirectDrawBuffer* indirect = nullptr;
	public:
		bool loadModel(AnthemSimpleToyRenderer* renderer,std::vector<AnthemUtlSimpleModelStruct> model, uint32_t cpuJobs);
		AnthemVertexBuffer* getVertexBuffer();
		AnthemIndexBuffer* getIndexBuffer();
		AnthemIndirectDrawBuffer* getIndirectBuffer();
	};
}