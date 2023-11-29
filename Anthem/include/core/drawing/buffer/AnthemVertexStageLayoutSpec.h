#pragma once
#include "../../base/AnthemBaseImports.h"

namespace Anthem::Core {
	struct AnthemVertexStageLayoutSpec {
		std::vector<VkVertexInputAttributeDescription> attributeDesc = {}
		VkPipelineVertexInputStateCreateInfo createInfo = {};
		map<int, VkVertexInputAttributeDescription> registeredDesc = {};
	};
}