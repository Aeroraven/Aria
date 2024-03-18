#include "../../../include/core/pipeline/AnthemShaderModuleBase.h"

namespace Anthem::Core {
    bool AnthemShaderModuleBase::createSingleShaderModule(AnthemLogicalDevice* device, std::vector<char>* shaderCode, std::optional<VkShaderModule>* shaderModule) {
        ANTH_ASSERT(shaderModule->has_value(), "Invalid Shader Module");
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode->size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode->data());
        if (vkCreateShaderModule(device->getLogicalDevice(), &createInfo, nullptr, &(shaderModule->value())) != VK_SUCCESS) {
            ANTH_LOGE("Failed to create shader module");
            return false;
        }
        return true;
    }
}