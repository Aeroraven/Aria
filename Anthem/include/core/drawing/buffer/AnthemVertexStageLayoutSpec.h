#pragma once
#include "../../base/AnthemBaseImports.h"

namespace Anthem::Core {
	struct AnthemVertexStageLayoutSpec {
		std::vector<VkVertexInputAttributeDescription> attributeDesc = {};
		std::vector<VkVertexInputBindingDescription> bindingDesc = {};
		VkPipelineVertexInputStateCreateInfo createInfo = {};
		std::map<int, VkVertexInputAttributeDescription> registeredDesc = {};
	};
}