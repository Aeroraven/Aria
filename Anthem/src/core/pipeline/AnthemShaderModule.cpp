#include "../../../include/core/pipeline/AnthemShaderModule.h"

namespace Anthem::Core{
    bool AnthemShaderModule::createShaderModules(AnthemLogicalDevice* device,const AnthemShaderFilePaths* filename){
        //Load Shader Code
        ANTH_LOGI("Loading Shader Code");
        auto vertexShaderCode = new std::vector<char>();
        auto fragShaderCode = new std::vector<char>();
        ANTH_ASSERT(filename->vertexShader.has_value(),"Vertex shader file path not specified");
        ANTH_ASSERT(filename->fragmentShader.has_value(),"Fragment shader file path not specified");
        this->readFile(filename->vertexShader.value(),vertexShaderCode);
        this->readFile(filename->fragmentShader.value(),fragShaderCode);

        //Specify Shader Module]
        ANTH_LOGI("Creating Shader Module");
        this->createSingleShaderModule(device,vertexShaderCode,&(this->shaderModules->vertexShaderModule));
        this->createSingleShaderModule(device,fragShaderCode,&(this->shaderModules->fragmentShaderModule));
        this->shaderModules->vertexShaderModuleCreated = true;
        return true;
    }
    bool AnthemShaderModule::createSingleShaderModule(AnthemLogicalDevice* device,std::vector<char>* shaderCode,std::optional<VkShaderModule>* shaderModule){
        ANTH_ASSERT(shaderModule->has_value() > 0,"Invalid Shader Module");
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode->size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode->data());
        if(vkCreateShaderModule(device->getLogicalDevice(),&createInfo,nullptr,&(shaderModule->value())) != VK_SUCCESS){
            ANTH_LOGE("Failed to create shader module");
            return false;
        }
        return true;
    }
    bool AnthemShaderModule::destroyShaderModules(AnthemLogicalDevice* device){
        vkDestroyShaderModule(device->getLogicalDevice(),this->shaderModules->vertexShaderModule.value(),nullptr);
        vkDestroyShaderModule(device->getLogicalDevice(),this->shaderModules->fragmentShaderModule.value(),nullptr);
        return true;
    }
    bool AnthemShaderModule::specifyShaderStageCreateInfo(std::vector<VkPipelineShaderStageCreateInfo>* shaderStageCreateInfo) const{
        ANTH_ASSERT(this->shaderModules->vertexShaderModuleCreated,"Shader module not created");

        VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
        vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageCreateInfo.module = this->shaderModules->vertexShaderModule.value();
        vertexShaderStageCreateInfo.pName = "main";
        shaderStageCreateInfo->push_back(vertexShaderStageCreateInfo);

        VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
        fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageCreateInfo.module = this->shaderModules->fragmentShaderModule.value();
        fragmentShaderStageCreateInfo.pName = "main";
        shaderStageCreateInfo->push_back(fragmentShaderStageCreateInfo);
        return true;
    }
}