#pragma once
#include "../base/AnthemBaseImports.h"
#include "AnthemShaderModule.h"
#include "../drawing/AnthemDescriptorPool.h"
#include "./AnthemDescriptorSetEntry.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "./AnthemPipelineBase.h"
namespace Anthem::Core{
    class AnthemComputePipeline:public virtual AnthemPipelineBase {
    public:
        bool pipelineCreated = false;
        bool prerequisiteInfoSpecified = false;
  
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
        const VkPipelineLayout* getPipelineLayout() const {
            ANTH_ASSERT(this->pipelineCreated, "Invalid pipeline");
            return &(this->pipelineLayout);
        }
    };
}