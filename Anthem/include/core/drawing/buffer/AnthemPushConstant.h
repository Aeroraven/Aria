#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "AnthemGeneralBufferBase.h"

namespace Anthem::Core {
	
	enum AnthemPushConstantStageFlags {
		AT_APCS_VERTEX = 1,
		AT_APCS_FRAGMENT = 2
	};

	class AnthemPushConstant {
	private:
		VkShaderStageFlags flags = 0;
		
	public:
		virtual uint32_t getSize() = 0;
		virtual bool enableShaderStage(AnthemPushConstantStageFlags stage);
		virtual VkPushConstantRange getRange();
	};
}