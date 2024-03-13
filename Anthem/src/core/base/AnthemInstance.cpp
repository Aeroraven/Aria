#include "../../../include/core/base/AnthemInstance.h"

namespace Anthem::Core{
    bool AnthemInstance::callResizeHandler(int w,int h){
        this->resizeHandler(w,h);\
        return true;
    }
    bool AnthemInstance::specifyResizeHandler(std::function<void(int,int)> handler){
        this->resizeHandler = handler;
        return true;
    }
    bool AnthemInstance::waitForFramebufferReady(){
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
        return true;
    }
    bool AnthemInstance::createWindow(){
        ANTH_ASSERT(config != nullptr, "Config not specified");
        auto w = config->APP_RESLOUTION_W;
        auto h = config->APP_RESLOUTION_H;
        ANTH_LOGI("Creating window with width ",w," and height ",h);
        if(glfwInit()==GLFW_FALSE){
            ANTH_LOGI("Failed to initialize GLFW");
            return false;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        this->window = glfwCreateWindow(w, h, config->APP_NAME, NULL, NULL);

        if(window==nullptr){
            ANTH_LOGI("Failed to create window");
            return false;
        }

        //Setup Resize Handler
        auto fbResizeCallback = [](GLFWwindow* window, int width, int height){
            auto app = reinterpret_cast<AnthemInstance*>(glfwGetWindowUserPointer(window));
            app->callResizeHandler(width,height);
        };
        glfwSetWindowUserPointer(window,this);
        glfwSetFramebufferSizeCallback(this->window,fbResizeCallback);
        
        return true;
    }
    bool AnthemInstance::destroyWindow(){
        ANTH_ASSERT(window != nullptr, "Window not specified");
        glfwDestroyWindow(window);
        glfwTerminate();
        return true;
    }
    bool AnthemInstance::startDrawLoop(std::function<void()> drawFunc){
        ANTH_ASSERT(window != nullptr, "Window not specified");
        ANTH_LOGI("Loop starts");
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();
            drawFunc();
        }
        return true;
    }
    bool AnthemInstance::createInstance(){
        ANTH_ASSERT(config != nullptr, "Config not specified");
        ANTH_LOGI("Creating instance");
        if(!glfwVulkanSupported()){
            ANTH_LOGI("Vulkan not supported");
            return false;
        }
        this->appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        this->appInfo.pApplicationName = config->APP_NAME;
        this->appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
        this->appInfo.pEngineName = config->ENGINE_NAME;
        this->appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        this->appInfo.apiVersion = VK_API_VERSION_1_2;

        this->createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        this->createInfo.pApplicationInfo = &this->appInfo;

        auto extensions = this->getRequiredExtensions();
        this->createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        this->createInfo.ppEnabledExtensionNames = extensions.data();

        if(config->VKCFG_ENABLE_VALIDATION_LAYERS){
            this->createInfo.enabledLayerCount = static_cast<uint32_t>(config->VKCFG_VALIDATION_LAYERS.size());
            this->createInfo.ppEnabledLayerNames = config->VKCFG_VALIDATION_LAYERS.data();
        }else{
            this->createInfo.enabledLayerCount = 0;
        }

        if(vkCreateInstance(&this->createInfo,nullptr,&this->instance)!=VK_SUCCESS){
            ANTH_LOGE("Failed to create instance");
            return false;
        }
        ANTH_LOGI("Instance created");
        return true;
    }
    bool AnthemInstance::destroyInstance(){
        ANTH_ASSERT(instance != nullptr, "Instance not specified");
        ANTH_LOGI("Destroying instance");
        vkDestroyInstance(instance,nullptr);
        return true;
    }
    std::vector<const char*> AnthemInstance::getRequiredExtensions(){
            
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions,glfwExtensions+glfwExtensionCount);
        if(config->VKCFG_ENABLE_VALIDATION_LAYERS){
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return extensions;
    }
    const VkInstanceCreateInfo* AnthemInstance::getCreateInfo() const{
        return &this->createInfo;
    }
    const void** AnthemInstance::getCreateInfoPNext(){
        return &(this->createInfo.pNext);
    }
    const GLFWwindow* AnthemInstance::getWindow() const{
        return this->window;
    }
    const VkInstance* AnthemInstance::getInstance() const{
        return &this->instance;
    }

}