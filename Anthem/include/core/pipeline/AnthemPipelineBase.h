#pragma once
#include "../base/AnthemBaseImports.h"
#include "AnthemShaderModule.h"
#include "../drawing/AnthemDescriptorPool.h"
#include "./AnthemDescriptorSetEntry.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../drawing/buffer/AnthemPushConstant.h"

namespace Anthem::Core {
	class AnthemPipelineBase :
	public virtual Util::AnthemUtlLogicalDeviceReqBase {
	protected:
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		VkPipelineLayout pipelineLayout = {};
		std::vector<VkDescriptorSetLayout> layouts;
	public:
		bool createPipelineLayoutCustomized(const std::vector<AnthemDescriptorSetEntry>& entry, const std::vector<AnthemPushConstant*> pushConsts);
	};
}