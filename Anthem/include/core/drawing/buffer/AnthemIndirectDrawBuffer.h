#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "AnthemGeneralBufferBase.h"


namespace Anthem::Core {
	class AnthemIndirectDrawBuffer : public AnthemGeneralBufferBase,
		public Util::AnthemUtlCommandBufferReqBase {

	protected:
		AnthemGeneralBufferProp indirect;
		std::vector<VkDrawIndexedIndirectCommand> indirectCmds;
		bool created = false;

	public:
		bool addIndirectDrawCommand(uint32_t instanceNum,uint32_t firstInstance, uint32_t vertexCount,uint32_t firstIndex,uint32_t vertexOffset);
		bool createBuffer();
		bool destroyBuffer();
		uint32_t virtual calculateBufferSize() override;
		const VkBuffer* getBuffer() const;
		uint32_t getNumCommands() const;
	};
}