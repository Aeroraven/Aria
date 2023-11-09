#include "../../../include/core/pipeline/AnthemShaderModule.h"

namespace Anthem::Core{
    bool AnthemShaderModule::createShaderModules(AnthemLogicalDevice* device,const AnthemShaderFilePaths* filename){
        //Load Shader Code
        ANTH_LOGI("Loading Shader Code");
        auto vertexShaderCode = new std::vector<char>();
        auto fragShaderCode = new std::vector<char>();
        auto geomShaderCode = new std::vector<char>();
        auto compShaderCode = new std::vector<char>();

        if(filename->vertexShader.has_value()){
            this->readFile(filename->vertexShader.value(),vertexShaderCode);
        }
        if(filename->fragmentShader.has_value()){
            this->readFile(filename->fragmentShader.value(),fragShaderCode);
        }
        if(filename->geometryShader.has_value()){
            this->readFile(filename->geometryShader.value(),geomShaderCode);
        }
        if(filename->computeShader.has_value()){
            this->readFile(filename->computeShader.value(),compShaderCode);
        }

        //Specify Shader Module
        ANTH_LOGI("Creating Shader Module");
        if(filename->vertexShader.has_value()){
            this->shaderModules->vertexShaderModule = std::make_optional<VkShaderModule>();
            this->createSingleShaderModule(device,vertexShaderCode,&(this->shaderModules->vertexShaderModule));
            ANTH_LOGI("Vertex shader loaded");
        }
        if(filename->fragmentShader.has_value()){
            this->shaderModules->fragmentShaderModule = std::make_optional<VkShaderModule>();
            this->createSingleShaderModule(device,fragShaderCode,&(this->shaderModules->fragmentShaderModule));
            ANTH_LOGI("Fragment shader loaded");
        }
        if(filename->geometryShader.has_value()){
            this->shaderModules->geometryShaderModule = std::make_optional<VkShaderModule>();
            this->createSingleShaderModule(device,geomShaderCode,&(this->shaderModules->geometryShaderModule));
            ANTH_LOGI("Geometry shader loaded");
        }
        if(filename->computeShader.has_value()){
            this->shaderModules->computeShaderModule = std::make_optional<VkShaderModule>();
            this->createSingleShaderModule(device,compShaderCode,&(this->shaderModules->computeShaderModule));
            ANTH_LOGI("Compute shader loaded");
        }
        this->shaderModules->vertexShaderModuleCreated = true;
        return true;
    }
    bool AnthemShaderModule::createSingleShaderModule(AnthemLogicalDevice* device,std::vector<char>* shaderCode,std::optional<VkShaderModule>* shaderModule){
        ANTH_ASSERT(shaderModule->has_value(),"Invalid Shader Module");
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
        if(this->shaderModules->vertexShaderModule.has_value()){
            vkDestroyShaderModule(device->getLogicalDevice(),this->shaderModules->vertexShaderModule.value(),nullptr);
        }
        if(this->shaderModules->fragmentShaderModule.has_value()){
            vkDestroyShaderModule(device->getLogicalDevice(),this->shaderModules->fragmentShaderModule.value(),nullptr);
        }
        if(this->shaderModules->geometryShaderModule.has_value()){
            vkDestroyShaderModule(device->getLogicalDevice(),this->shaderModules->geometryShaderModule.value(),nullptr);
        }
        if(this->shaderModules->computeShaderModule.has_value()){
            vkDestroyShaderModule(device->getLogicalDevice(),this->shaderModules->computeShaderModule.value(),nullptr);
        }
        return true;
    }
    bool AnthemShaderModule::specifyShaderStageCreateInfo(std::vector<VkPipelineShaderStageCreateInfo>* shaderStageCreateInfo) const{
        ANTH_ASSERT(this->shaderModules->vertexShaderModuleCreated,"Shader module not created");
        if(this->shaderModules->vertexShaderModule.has_value()){
            VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
            vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertexShaderStageCreateInfo.module = this->shaderModules->vertexShaderModule.value();
            vertexShaderStageCreateInfo.pName = "main";
            shaderStageCreateInfo->push_back(vertexShaderStageCreateInfo);
        }

        if(this->shaderModules->fragmentShaderModule.has_value()){
            VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
            fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragmentShaderStageCreateInfo.module = this->shaderModules->fragmentShaderModule.value();
            fragmentShaderStageCreateInfo.pName = "main";
            shaderStageCreateInfo->push_back(fragmentShaderStageCreateInfo);
        }

        if(this->shaderModules->geometryShaderModule.has_value()){
            VkPipelineShaderStageCreateInfo geometryShaderStageCreateInfo = {};
            geometryShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            geometryShaderStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            geometryShaderStageCreateInfo.module = this->shaderModules->geometryShaderModule.value();
            geometryShaderStageCreateInfo.pName = "main";
            shaderStageCreateInfo->push_back(geometryShaderStageCreateInfo);
        }

        if(this->shaderModules->computeShaderModule.has_value()){
            VkPipelineShaderStageCreateInfo computeShaderStageCreateInfo = {};
            computeShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            computeShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            computeShaderStageCreateInfo.module = this->shaderModules->computeShaderModule.value();
            computeShaderStageCreateInfo.pName = "main";
            shaderStageCreateInfo->push_back(computeShaderStageCreateInfo);
        }
        return true;
    }
}