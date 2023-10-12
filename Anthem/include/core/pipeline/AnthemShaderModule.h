#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlFileReaderBase.h"
#include "../base/AnthemLogicalDevice.h"

namespace Anthem::Core{

    struct AnthemShaderFilePaths{
        std::optional<std::string> vertexShader;
        std::optional<std::string> fragmentShader;
    };

    struct AnthemShaderModulesGroup{
        bool vertexShaderModuleCreated = false;
        std::optional<VkShaderModule> vertexShaderModule;
        std::optional<VkShaderModule> fragmentShaderModule;
    };

    class AnthemShaderModule: private Util::AnthemUtlFileReaderBase{
    private:
        ANTH_UNIQUE_PTR(AnthemShaderModulesGroup) shaderModules = std::make_unique<AnthemShaderModulesGroup>();
    public:
        bool createShaderModules(AnthemLogicalDevice* device,const AnthemShaderFilePaths* filename); 
        bool createSingleShaderModule(AnthemLogicalDevice* device,std::vector<char>* shaderCode,std::optional<VkShaderModule>* shaderModule);
        bool destroyShaderModules(AnthemLogicalDevice* device);
        bool specifyShaderStageCreateInfo(std::vector<VkPipelineShaderStageCreateInfo>* shaderStageCreateInfo);
    };
}