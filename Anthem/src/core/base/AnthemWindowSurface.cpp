#include "../../../include/core/base/AnthemWindowSurface.h"

namespace Anthem::Core{
    bool AnthemWindowSurface::createWindowSurface(const VkInstance* instance){
        auto result = glfwCreateWindowSurface(*instance,window,nullptr,&windowSurface);
        if(result != VK_SUCCESS){
            ANTH_LOGI("Failed to create window surface",result);
            return false;
        }
        ANTH_LOGI("Window surface created");
        return true;
    }
    AnthemWindowSurface::AnthemWindowSurface(GLFWwindow* window){
        this->window = window;
    }
    bool AnthemWindowSurface::destroyWindowSurface(const VkInstance* instance){
        ANTH_LOGI("Destroying window surface");
        vkDestroySurfaceKHR(*instance, windowSurface, nullptr);
        return true;
    }
    VkSurfaceKHR AnthemWindowSurface::getWindowSurface(){
        return this->windowSurface;
    }

    
}