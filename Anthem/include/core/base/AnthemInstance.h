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
    private:
        std::vector<const char*> getRequiredExtensions();
    };
}