#pragma once
#include "AnthemBaseImports.h"
#include "../utils/AnthemUtlAppConfigReqBase.h"

namespace Anthem::Core{
    class AnthemInstance:public Util::AnthemUtlConfigReqBase{
    private:
        GLFWwindow* window;
        VkApplicationInfo appInfo = {};
        VkInstanceCreateInfo createInfo = {};
        VkInstance instance;

        std::function<void(int,int)> resizeHandler = [](int,int){};
    public:
        bool virtual createWindow();
        bool virtual destroyWindow();
        bool virtual startDrawLoop(std::function<void()> drawFunc);

        bool virtual createInstance();
        bool virtual destroyInstance();

        const VkInstance* getInstance() const;
        const VkInstanceCreateInfo* getCreateInfo() const;
        const void** getCreateInfoPNext();
        const GLFWwindow* getWindow() const;

        bool virtual specifyResizeHandler(std::function<void(int,int)> handler);
        bool virtual callResizeHandler(int w,int h);
        bool virtual waitForFramebufferReady();
    private:
        std::vector<const char*> getRequiredExtensions();
        
    };
}