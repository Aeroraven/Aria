#pragma once
#include "../../base/AnthemBaseImports.h"

namespace Anthem::Core {
	class IAnthemVertexBufferAttrLayout {
	public:
		bool virtual getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc) = 0;
		bool virtual getInputAttrDescriptionInternal(std::vector<VkVertexInputAttributeDescription>* desc) = 0;
		bool virtual prepareVertexInputInfo(VkPipelineVertexInputStateCreateInfo* info) = 0;
	};
}