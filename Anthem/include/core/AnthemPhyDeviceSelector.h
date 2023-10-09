#pragma once
#include "AnthemDefs.h"
#include "AnthemLogger.h"

namespace Anthem::Core{
    struct AnthemPhyQueueFamilyIdx{
        std::optional<uint32_t> graphicsFamily;
    };

    class AnthemPhyDeviceSelector{
    private:
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx) physicalDeviceQueueFamily;
        
    public:
        bool virtual selectPhyDevice(VkInstance* instance);
        ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx) virtual findQueueFamilies(VkPhysicalDevice device);
        ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx) virtual getPhyDeviceQueueFamilyInfo();
        VkPhysicalDevice virtual getPhyDevice();
    };
}
