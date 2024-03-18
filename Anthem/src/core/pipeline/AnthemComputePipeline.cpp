#include "../../../include/core/pipeline/AnthemComputePipeline.h"

namespace Anthem::Core{
    const VkPipeline* AnthemComputePipeline::getPipeline() const{
        ANTH_ASSERT(this->pipelineCreated,"Invalid pipeline");
        return &(this->pipeline);
    }
    bool AnthemComputePipeline::createPipeline(){
        this->shaderModule->specifyShaderStageCreateInfo(&this->shaderStageCreateInfo);
        ANTH_ASSERT(this->shaderStageCreateInfo.size()==1,"There can be only one shader stage");
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.layout = this->pipelineLayout;
        pipelineCreateInfo.stage = this->shaderStageCreateInfo.at(0);

        auto res = vkCreateComputePipelines(this->logicalDevice->getLogicalDevice(),VK_NULL_HANDLE,1,&pipelineCreateInfo,nullptr,&this->pipeline);
        if(res!=VK_SUCCESS){
            ANTH_LOGE("Failed to create compute pipeline");
        }
        ANTH_LOGI("Compute pipeline created");
        pipelineCreated = true;
        return true;
    }
    bool AnthemComputePipeline::specifyShaderModule(const AnthemShaderModule* shaderModule){
        this->shaderModule = shaderModule;
        return true;
    }
    
    bool AnthemComputePipeline::destroyPipeline() {
        ANTH_LOGI("Destroying compute pipeline");
        vkDestroyPipeline(this->logicalDevice->getLogicalDevice(), this->pipeline, nullptr);
        this->pipelineCreated = false;
        return true;
    }
    bool AnthemComputePipeline::destroyPipelineLayout() {
        ANTH_LOGI("Destroying pipeline layout");
        vkDestroyPipelineLayout(this->logicalDevice->getLogicalDevice(), this->pipelineLayout, nullptr);
        return true;
    }
}