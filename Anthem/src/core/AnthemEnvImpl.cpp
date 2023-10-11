#include "../../include/core/base/AnthemEnvImpl.h"
#include "../../include/core/base/AnthemConfig.h"
#include "../../include/core/base/AnthemLogger.h"
#include "../../include/core/base/AnthemDefs.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

namespace Anthem{
    namespace Core{
        AnthemEnvImpl::AnthemEnvImpl(ANTH_SHARED_PTR(AnthemConfig) cfg){
            this->cfg = cfg;
            this->valLayer = ANTH_MAKE_SHARED(AnthemValLayer)(cfg);
        }
        bool AnthemEnvImpl::createWindow(int w,int h){
            ANTH_LOGI("Creating window with width ",w," and height ",h);
            if(glfwInit()==GLFW_FALSE){
                ANTH_LOGI("Failed to initialize GLFW");
                return false;
            }
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            this->window = glfwCreateWindow(w, h, cfg->APP_NAME, NULL, NULL);
            this->windowSurface = ANTH_MAKE_SHARED(AnthemWindowSurface)(window);
            
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
            ANTH_ASSERT(valLayer->checkValidLayerSupport(),"Validation layers not supported");

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
                valLayer->fillingPointerData(&(createInfo.pNext));
            }else{
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = nullptr; 
            }

            //Create Instance
            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
            ANTH_LOGI_IF(result==VK_SUCCESS,"Instance created successfully", "Failed to create instance");
        }
        void AnthemEnvImpl::destroyEnv(){
            ANTH_LOGI("Destroying environment");
            //Destroy Swap Chain Stuffs
            swapChain->destroySwapChainImageViews(this->logicalDevice.get());
            swapChain->destroySwapChain(this->logicalDevice.get());

            //Destroy Device & Layers
            logicalDevice->destroyLogicalDevice(&instance);
            valLayer->destroyDebugMsgLayer(&instance);
            windowSurface->destroyWindowSurface(&instance);

            //Destroy Instance
            vkDestroyInstance(this->instance, nullptr);
            glfwDestroyWindow(this->window);
            glfwTerminate();
        }
        void AnthemEnvImpl::init(){
            ANTH_LOGI(__PRETTY_FUNCTION__);
            //Startup
            this->createWindow(cfg->APP_RESLOUTION_W,cfg->APP_RESLOUTION_H);
            this->initSwapChain();
            valLayer->createDebugMsgLayerInfo();
            this->createInstance();

            valLayer->createDebugMsgLayer(&instance);
            windowSurface->createWindowSurface(&instance);

            //Selecting Physical Device
            phyDeviceSelector = ANTH_MAKE_SHARED(AnthemPhyDeviceSelector)(windowSurface,swapChain);
            phyDeviceSelector->selectPhyDevice(&instance,this->windowSurface);
            phyDevice = ANTH_MAKE_SHARED(AnthemPhyDevice)();
            phyDeviceSelector->getPhyDevice(this->phyDevice.get());

            //Creating Logical Device
            logicalDeviceSelector = ANTH_MAKE_SHARED(AnthemLogicalDeviceSelector)(this->phyDevice.get());
            logicalDeviceSelector->createLogicalDevice();
            logicalDevice = ANTH_MAKE_SHARED(AnthemLogicalDevice)();
            logicalDeviceSelector->getLogicalDevice(logicalDevice.get());

            //Prepare Swap Chain
            swapChain->specifySwapChainDetails(this->phyDevice.get(),this->window);
            swapChain->createSwapChain(this->logicalDevice.get(),this->phyDevice.get());
            swapChain->retrieveSwapChainImages(this->logicalDevice.get());
            swapChain->createSwapChainImageViews(this->logicalDevice.get());
        }
        void AnthemEnvImpl::run(){
            this->init();
            this->drawLoop();
            this->destroyEnv();
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
        void AnthemEnvImpl::initSwapChain(){
            this->swapChain = ANTH_MAKE_SHARED(AnthemSwapChain)(this->windowSurface);
            
        }
    }
}



