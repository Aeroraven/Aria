#pragma once
#include "AnthemDefs.h"
#include "AnthemLogger.h"
#include "AnthemWindowSurface.h"
#include "AnthemSwapChain.h"

namespace Anthem::Core{
    struct AnthemPhyQueueFamilyIdx{
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    class AnthemPhyDeviceSelector{
    private:
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx) physicalDeviceQueueFamily;
        ANTH_SHARED_PTR(AnthemWindowSurface) surface;
        ANTH_SHARED_PTR(AnthemSwapChain)  swapChain;
        std::vector<const char*> requiredDeviceSupportedExtension = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        
    public:
        bool virtual selectPhyDevice(VkInstance* instance, ANTH_SHARED_PTR(AnthemWindowSurface) surface);
        bool virtual checkDeviceExtensionSupport(VkPhysicalDevice device);
        ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx) virtual findQueueFamilies(VkPhysicalDevice device,ANTH_SHARED_PTR(AnthemWindowSurface) surface);
        ANTH_SHARED_PTR(AnthemPhyQueueFamilyIdx) virtual getPhyDeviceQueueFamilyInfo();
        VkPhysicalDevice virtual getPhyDevice();
        const std::vector<const char*>* const getRequiredDevSupportedExts() const;
        void virtual getDeviceFeature(VkPhysicalDeviceFeatures& outFeatures);
        AnthemPhyDeviceSelector(ANTH_SHARED_PTR(AnthemWindowSurface) surface,ANTH_SHARED_PTR(AnthemSwapChain) swapChain);
    };
}
