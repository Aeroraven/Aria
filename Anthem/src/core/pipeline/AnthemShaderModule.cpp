#include "../../../include/core/pipeline/AnthemShaderModule.h"

namespace Anthem::Core{
    bool AnthemShaderModule::createShaderModules(AnthemLogicalDevice* device,const AnthemShaderFilePaths* filename){
        //Load Shader Code

        auto createShaderSingle = [&](const std::optional<std::string>& path, std::optional<VkShaderModule>& shader,const char* stageName) {
            auto code = new std::vector<char>();
            if (path.has_value()) {
                this->readFile(path.value(), code);
                shader = std::make_optional<VkShaderModule>();
                this->createSingleShaderModule(device, code, &shader);
                ANTH_LOGV(stageName, " shader loaded.");
            }
        };

        createShaderSingle(filename->vertexShader, shaderModules->vertexShaderModule, "Vertex");
        createShaderSingle(filename->fragmentShader, shaderModules->fragmentShaderModule, "Fragment");
        createShaderSingle(filename->geometryShader, shaderModules->geometryShaderModule, "Geometry");
        createShaderSingle(filename->computeShader, shaderModules->computeShaderModule, "Compute");
        createShaderSingle(filename->tessControlShader, shaderModules->tessControlShader, "Tessellation Control");
        createShaderSingle(filename->tessEvalShader, shaderModules->tessEvalShader, "Tessellation Evaluation");
        createShaderSingle(filename->taskShader, shaderModules->taskShader, "Task");
        createShaderSingle(filename->meshShader, shaderModules->meshShader, "Mesh");

        this->shaderModules->vertexShaderModuleCreated = true;
        ANTH_LOGI("Shader code loaded");

        return true;
    }
    
    bool AnthemShaderModule::destroyShaderModules(AnthemLogicalDevice* device){

        auto destroyShaderSingle = [&](std::optional<VkShaderModule>& shader) {
            if (shader.has_value()) {
                vkDestroyShaderModule(device->getLogicalDevice(),shader.value(), nullptr);
            }
        };

        destroyShaderSingle(shaderModules->vertexShaderModule);
        destroyShaderSingle(shaderModules->fragmentShaderModule);
        destroyShaderSingle(shaderModules->geometryShaderModule);
        destroyShaderSingle(shaderModules->computeShaderModule);
        destroyShaderSingle(shaderModules->tessControlShader);
        destroyShaderSingle(shaderModules->tessEvalShader);
        destroyShaderSingle(shaderModules->taskShader);
        destroyShaderSingle(shaderModules->meshShader);

        return true;
    }
    bool AnthemShaderModule::specifyShaderStageCreateInfo(std::vector<VkPipelineShaderStageCreateInfo>* shaderStageCreateInfo) const{
        ANTH_ASSERT(this->shaderModules->vertexShaderModuleCreated,"Shader module not created");
        
        auto specifyInfoSingle = [&](std::optional<VkShaderModule>& shader, VkShaderStageFlagBits stage) {
            if (shader.has_value()) {
                VkPipelineShaderStageCreateInfo cInfo{};
                cInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                cInfo.stage = stage;
                cInfo.module = shader.value();
                cInfo.pName = "main";
                shaderStageCreateInfo->push_back(cInfo);
            }
        };
        
        specifyInfoSingle(shaderModules->vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
        specifyInfoSingle(shaderModules->fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
        specifyInfoSingle(shaderModules->geometryShaderModule, VK_SHADER_STAGE_GEOMETRY_BIT);
        specifyInfoSingle(shaderModules->computeShaderModule, VK_SHADER_STAGE_COMPUTE_BIT);
        specifyInfoSingle(shaderModules->tessControlShader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        specifyInfoSingle(shaderModules->tessEvalShader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        specifyInfoSingle(shaderModules->taskShader, VK_SHADER_STAGE_TASK_BIT_EXT);
        specifyInfoSingle(shaderModules->meshShader, VK_SHADER_STAGE_MESH_BIT_EXT);

        return true;
    }
}