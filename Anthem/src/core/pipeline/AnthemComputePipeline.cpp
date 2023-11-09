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
        ANTH_LOGE("Compute pipeline created");
        return true;
    }
    bool AnthemComputePipeline::specifyShaderModule(const AnthemShaderModule* shaderModule){
        this->shaderModule = shaderModule;
        return true;
    }
    bool AnthemComputePipeline::createPipelineLayoutCustomized(const std::vector<AnthemDescriptorSetEntry>& entry){
        this->pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        this->pipelineLayoutCreateInfo.pNext = nullptr;
        this->pipelineLayoutCreateInfo.flags = 0;
        for(const auto& p:entry){
            if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER){
                p.descPool->appendSamplerDescriptorLayoutIdx(&layouts,p.inTypeIndex);
                ANTH_LOGI("Sampler:",p.inTypeIndex);
            }else if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER){
                p.descPool->appendUniformDescriptorLayoutIdx(&layouts,p.inTypeIndex);
                ANTH_LOGI("Uniform:",p.inTypeIndex);
            }else if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SHADER_STORAGE_BUFFER){
                p.descPool->appendSsboDescriptorLayoutIdx(&layouts,p.inTypeIndex);
                ANTH_LOGI("SSBO:",p.inTypeIndex);
            }else{
                ANTH_LOGE("Invalid layout type");
            }
        }

        for(auto x:layouts){
            ANTH_LOGI("Layouts Are:",(long long)(x));
        }

        this->pipelineLayoutCreateInfo.pSetLayouts = layouts.data();
        this->pipelineLayoutCreateInfo.setLayoutCount = layouts.size();
        ANTH_LOGI("Specified pipeline layout",layouts.size());

        //Create Layout
        auto result = vkCreatePipelineLayout(this->logicalDevice->getLogicalDevice(),&(this->pipelineLayoutCreateInfo),nullptr,&(this->pipelineLayout));
        if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to create pipeline layout",result);
            return false;
        }
        ANTH_LOGI("Pipeline layout created");
        return true;
    }
}