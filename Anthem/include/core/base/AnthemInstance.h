#pragma once
#include "AnthemBaseImports.h"
#include "../utils/AnthemUtlAppConfigReqBase.h"

namespace Anthem::Core{
    class AnthemInstance:public Util::AnthemUtlConfigReqBase{
    private:
        GLFWwindow* window = nullptr;
        VkApplicationInfo appInfo = {};
        VkInstanceCreateInfo createInfo = {};
        VkInstance instance = nullptr;

        std::function<void(int, int)> resizeHandler = [](int, int) {};
        std::function<void(int, int, int)> mouseHandler = [](int, int, int) {};
        std::function<void(int, int, int, int)> keyboardHandler = [](int, int, int, int) {};
        std::function<void(double, double)> mouseMoveHandler = [](double, double) {};
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

        bool virtual specifyResizeHandler(std::function<void(int, int)> handler);
        bool virtual specifyMouseHandler(std::function<void(int, int, int)> handler);
        bool virtual specifyKeyHandler(std::function<void(int, int, int, int)> handler);
        bool virtual specifyMouseMoveHandler(std::function<void(double, double)> handler);
        bool virtual callResizeHandler(int w,int h);
        bool virtual callMouseHander(int a, int b, int c);
        bool virtual callKeyboardHander(int a, int b, int c,int d);
        bool virtual callMouseMoveHander(double a, double b);
        bool virtual waitForFramebufferReady();
    private:
        std::vector<const char*> getRequiredExtensions();
   
    };
}