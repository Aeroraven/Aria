#pragma once
#include "../../base/AnthemBaseImports.h"
#include "AnthemVertexStageLayoutSpec.h"

namespace Anthem::Core {
	class IAnthemVertexBufferAttrLayout {
	public:
		bool virtual getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc, uint32_t bindLoc) = 0;
		bool virtual getInputAttrDescriptionInternal(std::vector<VkVertexInputAttributeDescription>* desc, uint32_t bindLoc) = 0;

		bool virtual prepareVertexInputInfo(VkPipelineVertexInputStateCreateInfo* info, uint32_t bindLoc) = 0;
		//bool virtual prepareVertexInputInfo2(VkVertexInputBindingDescription* outBindingDesc, std::vector<VkVertexInputAttributeDescription>* outAttrDesc, uint32_t bindLoc) = 0;

		bool virtual updateLayoutSpecification(AnthemVertexStageLayoutSpec* spec, uint32_t bindLoc) = 0;
	};
}