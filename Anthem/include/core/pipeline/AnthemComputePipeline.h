#pragma once
#include "../base/AnthemBaseImports.h"
#include "AnthemShaderModule.h"
#include "../drawing/AnthemDescriptorPool.h"
#include "./AnthemDescriptorSetEntry.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"

namespace Anthem::Core{
    class AnthemComputePipeline:public virtual Util::AnthemUtlLogicalDeviceReqBase{
    public:
        bool pipelineCreated = false;

        bool prerequisiteInfoSpecified = false;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        VkPipelineLayout pipelineLayout = {};
        std::vector<VkDescriptorSetLayout> layouts;

        VkPipeline pipeline = nullptr;
        VkComputePipelineCreateInfo pipelineCreateInfo = {};
        const AnthemShaderModule* shaderModule = nullptr;

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfo = {};

        bool specifyShaderModule(const AnthemShaderModule* shaderModule);
        bool createPipelineLayoutCustomized(const std::vector<AnthemDescriptorSetEntry>& entry);
        bool createPipeline();
        const VkPipeline* getPipeline() const;

        bool destroyPipeline();
        bool destroyPipelineLayout();
    };
}