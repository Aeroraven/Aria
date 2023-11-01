#include "../../../include/core/pipeline/AnthemShaderModule.h"

namespace Anthem::Core{
    bool AnthemShaderModule::createShaderModules(AnthemLogicalDevice* device,const AnthemShaderFilePaths* filename){
        //Load Shader Code
        ANTH_LOGI("Loading Shader Code");
        auto vertexShaderCode = new std::vector<char>();
        auto fragShaderCode = new std::vector<char>();
        auto geomShaderCode = new std::vector<char>();
        ANTH_ASSERT(filename->vertexShader.has_value(),"Vertex shader file path not specified");
        ANTH_ASSERT(filename->fragmentShader.has_value(),"Fragment shader file path not specified");
        this->readFile(filename->vertexShader.value(),vertexShaderCode);
        this->readFile(filename->fragmentShader.value(),fragShaderCode);
        if(filename->geometryShader.has_value()){
            this->readFile(filename->geometryShader.value(),geomShaderCode);
        }


        //Specify Shader Module
        ANTH_LOGI("Creating Shader Module");
        this->shaderModules->vertexShaderModule = std::make_optional<VkShaderModule>();
        this->createSingleShaderModule(device,vertexShaderCode,&(this->shaderModules->vertexShaderModule));
        ANTH_LOGI("Vertex shader loaded");
        this->shaderModules->fragmentShaderModule = std::make_optional<VkShaderModule>();
        this->createSingleShaderModule(device,fragShaderCode,&(this->shaderModules->fragmentShaderModule));
        ANTH_LOGI("Fragment shader loaded");
        if(filename->geometryShader.has_value()){
            this->shaderModules->geometryShaderModule = std::make_optional<VkShaderModule>();
            this->createSingleShaderModule(device,geomShaderCode,&(this->shaderModules->geometryShaderModule));
            ANTH_LOGI("Geometry shader loaded");
        }
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
        if(this->shaderModules->geometryShaderModule.has_value()){
            vkDestroyShaderModule(device->getLogicalDevice(),this->shaderModules->geometryShaderModule.value(),nullptr);
        }
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

        if(this->shaderModules->geometryShaderModule.has_value()){
            VkPipelineShaderStageCreateInfo geometryShaderStageCreateInfo = {};
            geometryShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            geometryShaderStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            geometryShaderStageCreateInfo.module = this->shaderModules->geometryShaderModule.value();
            geometryShaderStageCreateInfo.pName = "main";
            shaderStageCreateInfo->push_back(geometryShaderStageCreateInfo);
        }
        return true;
    }
}