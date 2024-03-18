#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlFileReaderBase.h"
#include "../base/AnthemLogicalDevice.h"
#include "./AnthemShaderModuleBase.h"

namespace Anthem::Core{

    struct AnthemShaderFilePaths{
        std::optional<std::string> vertexShader;
        std::optional<std::string> fragmentShader;
        std::optional<std::string> geometryShader;
        std::optional<std::string> computeShader;
        std::optional<std::string> tessControlShader;
        std::optional<std::string> tessEvalShader;
        std::optional<std::string> meshShader;
        std::optional<std::string> taskShader;
    };

    struct AnthemShaderModulesGroup{
        bool vertexShaderModuleCreated = false;
        std::optional<VkShaderModule> vertexShaderModule = std::nullopt;
        std::optional<VkShaderModule> fragmentShaderModule = std::nullopt;
        std::optional<VkShaderModule> geometryShaderModule = std::nullopt;
        std::optional<VkShaderModule> computeShaderModule = std::nullopt;
        std::optional<VkShaderModule> tessControlShader = std::nullopt;
        std::optional<VkShaderModule> tessEvalShader = std::nullopt;
        std::optional<VkShaderModule> meshShader = std::nullopt;
        std::optional<VkShaderModule> taskShader = std::nullopt;
    };

    class AnthemShaderModule: 
    private Util::AnthemUtlFileReaderBase,
    private AnthemShaderModuleBase{
    private:
        ANTH_UNIQUE_PTR(AnthemShaderModulesGroup) shaderModules = std::make_unique<AnthemShaderModulesGroup>();
    public:
        bool createShaderModules(AnthemLogicalDevice* device,const AnthemShaderFilePaths* filename); 
        
        bool destroyShaderModules(AnthemLogicalDevice* device);
        bool specifyShaderStageCreateInfo(std::vector<VkPipelineShaderStageCreateInfo>* shaderStageCreateInfo) const;
    };
}