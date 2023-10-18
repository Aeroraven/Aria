#pragma once
#include "AnthemDefs.h"
#include "AnthemLogger.h"
#include "AnthemWindowSurface.h"
#include "AnthemSwapChain.h"
#include "AnthemPhyDevice.h"

namespace Anthem::Core{
    class AnthemPhyDeviceSelector{
    private:
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        ANTH_UNSAFE_PTR(AnthemPhyQueueFamilyIdx) physicalDeviceQueueFamily;

        AnthemWindowSurface* surface;
        AnthemSwapChain*  swapChain;
        std::vector<const char*> requiredDeviceSupportedExtension = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        
    public:
        bool virtual selectPhyDevice(const VkInstance* instance, AnthemWindowSurface* surface);
        bool virtual checkDeviceExtensionSupport(VkPhysicalDevice device);
        ANTH_UNSAFE_PTR(AnthemPhyQueueFamilyIdx) findQueueFamilies(VkPhysicalDevice device,AnthemWindowSurface* surface);
        bool virtual getPhyDevice(AnthemPhyDevice* phyDevice);
        const std::vector<const char*>* const getRequiredDevSupportedExts() const;
        void virtual getDeviceFeature(VkPhysicalDeviceFeatures& outFeatures);
        AnthemPhyDeviceSelector(AnthemWindowSurface* surface,AnthemSwapChain* swapChain);
    };
}
