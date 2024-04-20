#include "../../../include/core/base/AnthemValLayer.h"

namespace Anthem::Core{
    bool AnthemValLayer::checkValidLayerSupport(){
        if(cfg->vkcfgEnableValidationLayers==false){
            return true;
        }
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for(const char* layerName : cfg->vkcfgValidationLayers){
            bool layerFound = false;
            for(const auto& layerProperties : availableLayers){
                if(strcmp(layerName, layerProperties.layerName)==0){
                    layerFound = true;
                    break;
                }
            }
            if(!layerFound){
                ANTH_LOGI("Layer ",layerName," not found");
                return false;
            }
        }
        return true;
    }
    bool AnthemValLayer::destroyDebugMsgLayer(const VkInstance* instance){
        if(!cfg->vkcfgEnableValidationLayers){
            return true;
        }
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkDestroyDebugUtilsMessengerEXT");
        if(func!=nullptr){
            ANTH_LOGI("Debug Messenger Destruction Function Found");
            func(*instance, debugMessenger, nullptr);
        }else{
            ANTH_LOGI("Debug Messenger Destruction Function Not Found");
            return false;
        }
        return true;
    }
    bool AnthemValLayer::createDebugMsgLayer(const VkInstance* instance){
        if(!cfg->vkcfgEnableValidationLayers){
            return true;
        }
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");
        if(func!=nullptr){
            ANTH_LOGI("Debug Messenger Creation Function Found");
            return func(*instance, &debugCreateInfo, nullptr, &debugMessenger)==VK_SUCCESS;
        }else{
            ANTH_LOGI("Debug Messenger Creation Function Not Found");
            return false;
        }
    }
    bool AnthemValLayer::fillingPointerData(const void** p){
        if(!cfg->vkcfgEnableValidationLayers){
            return true;
        }
        *p = &debugCreateInfo;
        return true;
    }
    bool AnthemValLayer::createDebugMsgLayerInfo(){
        if(!cfg->vkcfgEnableValidationLayers){
            return true;
        }
        debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = nullptr;
        return true;
    }
    AnthemValLayer::AnthemValLayer(AnthemConfig* cfg){
        this->cfg = cfg;
    }
}