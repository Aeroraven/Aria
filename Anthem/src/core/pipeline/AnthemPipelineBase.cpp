#include "../../../include/core/pipeline/AnthemPipelineBase.h"

namespace Anthem::Core {
    bool AnthemPipelineBase::createPipelineLayoutCustomized(const std::vector<AnthemDescriptorSetEntry>& entry) {
        this->pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        this->pipelineLayoutCreateInfo.pNext = nullptr;
        this->pipelineLayoutCreateInfo.flags = 0;
        for (const auto& p : entry) {
            if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER) {
                p.descPool->appendSamplerDescriptorLayoutIdx(&layouts, p.inTypeIndex);
            }
            else if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER) {
                p.descPool->appendUniformDescriptorLayoutIdx(&layouts, p.inTypeIndex);
            }
            else if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SHADER_STORAGE_BUFFER) {
                p.descPool->appendSsboDescriptorLayoutIdx(&layouts, p.inTypeIndex);
            }
            else if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_STORAGE_IMAGE) {
                p.descPool->appendStorageImageDescriptorLayoutIdx(&layouts, p.inTypeIndex);
            }
            else if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_ACC_STRUCT) {
                p.descPool->appendAccStructDescriptorLayoutIdx(&layouts, p.inTypeIndex);
            }
            else {
                ANTH_LOGE("Invalid layout type");
            }
        }
        this->pipelineLayoutCreateInfo.pSetLayouts = layouts.data();
        this->pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        //Create Layout
        auto result = vkCreatePipelineLayout(this->logicalDevice->getLogicalDevice(), &(this->pipelineLayoutCreateInfo), nullptr, &(this->pipelineLayout));
        if (result != VK_SUCCESS) {
            ANTH_LOGE("Failed to create pipeline layout", result);
            return false;
        }
        ANTH_LOGI("Pipeline layout created");
        return true;
    }
}