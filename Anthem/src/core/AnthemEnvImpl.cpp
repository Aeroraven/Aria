#include "../../include/core/AnthemEnvImpl.h"
#include "../../include/core/AnthemConfig.h"
#include "../../include/core/AnthemLogger.h"
#include "../../include/core/AnthemDefs.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

namespace Anthem{
    namespace Core{
        AnthemEnvImpl::AnthemEnvImpl(ANTH_SHARED_PTR(AnthemConfig) cfg){
            this->cfg = cfg;
        }
        bool AnthemEnvImpl::createWindow(int w,int h){
            ANTH_LOGI("Creating window with width ",w," and height ",h);
            if(glfwInit()==GLFW_FALSE){
                ANTH_LOGI("Failed to initialize GLFW");
                return false;
            }
            this->window = glfwCreateWindow(w, h, cfg->APP_NAME, NULL, NULL);
            
            if(window==nullptr){
                ANTH_LOGI("Failed to create window");
                return false;
            }
            return true;
        }
        void AnthemEnvImpl::drawLoop(){
            while(!glfwWindowShouldClose(this->window)){
                glfwSwapBuffers(this->window);
                glfwPollEvents();
            }
        }
        void AnthemEnvImpl::createInstance(){
            //Checking for layer support
            if(cfg->VKCFG_ENABLE_VALIDATION_LAYERS){
                ANTH_ASSERT(this->checkValidLayerSupport(),"Validation layers not supported");
            }

            //Filling App Info
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = cfg->APP_NAME;
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = cfg->ENGINE_NAME;
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            //Filling Create Info
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            //Finding Extensions
            auto extensions = this->getRequiredExtensions();

            //Specify Extensions
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            //Specify Val Layers
            if(cfg->VKCFG_ENABLE_VALIDATION_LAYERS){
                createInfo.enabledLayerCount = static_cast<uint32_t>(cfg->VKCFG_VALIDATION_LAYERS.size());
                createInfo.ppEnabledLayerNames = cfg->VKCFG_VALIDATION_LAYERS.data();
                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else{
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = nullptr; 
            }

            //Create Instance
            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
            ANTH_LOGI_IF(result==VK_SUCCESS,"Instance created successfully", "Failed to create instance");
        }
        void AnthemEnvImpl::destroyEnv(){
            ANTH_LOGI("Destroying environment");
            if(cfg->VKCFG_ENABLE_VALIDATION_LAYERS){
                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
                if(func!=nullptr){
                    func(instance, debugMessenger, nullptr);
                }
            }
            vkDestroyInstance(this->instance, nullptr);
            glfwDestroyWindow(this->window);
            glfwTerminate();
        }
        void AnthemEnvImpl::run(){
            //Startup
            this->createWindow(cfg->APP_RESLOUTION_W,cfg->APP_RESLOUTION_H);
            //Init
            this->createDebugMsgLayerInfo();
            this->createInstance();
            this->createDebugMsgLayer();
            this->selectPhyDevice();
            //Run
            this->drawLoop();
            this->destroyEnv();
        }
        bool AnthemEnvImpl::checkValidLayerSupport(){
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            for(const char* layerName : cfg->VKCFG_VALIDATION_LAYERS){
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
        std::vector<const char*> AnthemEnvImpl::getRequiredExtensions(){
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
            if(cfg->VKCFG_ENABLE_VALIDATION_LAYERS){
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            return extensions;
        }
        bool AnthemEnvImpl::createDebugMsgLayerInfo(){
            if(!cfg->VKCFG_ENABLE_VALIDATION_LAYERS){
                return true;
            }
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;
            debugCreateInfo.pUserData = nullptr;
            return true;
        }

        bool AnthemEnvImpl::createDebugMsgLayer(){
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if(func!=nullptr){
                VkResult result = func(instance, &debugCreateInfo, nullptr, &debugMessenger);
                ANTH_LOGI_IF(result==VK_SUCCESS,"Debug messenger created successfully", "Failed to create debug messenger");
                return result==VK_SUCCESS;
            }
            else{
                ANTH_LOGI("Failed to load vkCreateDebugUtilsMessengerEXT");
                return false;
            }
        }

        bool AnthemEnvImpl::selectPhyDevice(){
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
            ANTH_ASSERT(deviceCount!=0,"Failed to find GPUs with support");
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            const auto checkDeviceSuitable = [](VkPhysicalDevice device){
                VkPhysicalDeviceProperties deviceProperties;
                VkPhysicalDeviceFeatures deviceFeatures;
                vkGetPhysicalDeviceProperties(device, &deviceProperties);
                vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
                ANTH_LOGI("Geometry:",deviceFeatures.geometryShader);
                ANTH_LOGI("",deviceProperties.limits.m)
                ANTH_LOGI("Max Image D2D:",deviceProperties.limits.maxImageDimension2D);
                return deviceProperties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            };

            for(const auto& device : devices){
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(device, &deviceProperties);
                ANTH_LOGI("Found GPU: ",deviceProperties.deviceName," ApiVer:",deviceProperties.apiVersion);
                checkDeviceSuitable(device);
            }
            return true;
        }
    }
}



