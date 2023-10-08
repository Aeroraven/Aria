#pragma once
#include "AnthemEnv.h"
#include "AnthemDefs.h"
#include "AnthemConfig.h"
#include "AnthemLogger.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <memory>

namespace Anthem{
    namespace Core{
        class AnthemEnvImpl: public AnthemEnv {
        private: //Member Variables
            GLFWwindow* window;
            VkApplicationInfo appInfo = {};
            VkInstanceCreateInfo createInfo = {};
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            ANTH_SHARED_PTR(AnthemConfig) cfg;

        public: //Member Functions
            AnthemEnvImpl(ANTH_SHARED_PTR(AnthemConfig) cfg);
            bool virtual createWindow(int w,int h) override;
            void virtual drawLoop() override;
            void virtual createInstance() override;
            void virtual destroyEnv() override;
            void virtual run() override; 
            
            bool virtual checkValidLayerSupport();
            std::vector<const char*> virtual getRequiredExtensions();
            bool virtual createDebugMsgLayerInfo();
            bool virtual createDebugMsgLayer();
            bool virtual selectPhyDevice();

        public: //Static
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData){
                    ANTH_LOGW("Validation Layer: ",pCallbackData->pMessage);
                    return VK_FALSE;
            }
        };
    }
}


