#pragma once
#include "AnthemConfig.h"
#include "AnthemDefs.h"
#include "AnthemLogger.h"

namespace Anthem::Core{
    class AnthemValLayer{
    private:
        AnthemConfig* cfg;
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        VkDebugUtilsMessengerEXT debugMessenger;
    public:
        AnthemValLayer(AnthemConfig* cfg);
        virtual bool checkValidLayerSupport();
        virtual bool createDebugMsgLayerInfo();
        virtual bool createDebugMsgLayer(const VkInstance* instance);
        virtual bool destroyDebugMsgLayer(const VkInstance* instance);
        virtual bool fillingPointerData(const void** p);
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData) {
                ANTH_LOGW("Validation Layer: ",pCallbackData->pMessage);
                return VK_FALSE;
        }

    };
}
