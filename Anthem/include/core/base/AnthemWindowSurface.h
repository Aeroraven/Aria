#pragma once
#include "AnthemDefs.h"
#include "AnthemLogger.h"

namespace Anthem::Core{
    class AnthemWindowSurface{
    private:
        VkSurfaceKHR windowSurface = VK_NULL_HANDLE;
        GLFWwindow* window;
    public:
        AnthemWindowSurface(GLFWwindow* window);
        bool virtual createWindowSurface(const VkInstance* instance);
        bool virtual destroyWindowSurface(const VkInstance* instance);
        VkSurfaceKHR virtual getWindowSurface();
    };
}