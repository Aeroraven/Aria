#pragma once
#include "AnthemDefs.h"
#include "AnthemLogger.h"

namespace Anthem::Core{
    struct AnthemPhyQueueFamilyIdx{
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };


    class AnthemPhyDevice{
    private:
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceFeatures deviceFeatures = {};
        VkPhysicalDeviceProperties deviceProperties = {};
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};
        AnthemPhyQueueFamilyIdx queueFamilyIdx = {};
        bool deviceSpecified = false;
        std::vector<const char*> requiredDeviceSupportedExtension = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        
    public:
        AnthemPhyDevice(); 
        bool virtual specifyDevice(VkPhysicalDevice device,AnthemPhyQueueFamilyIdx* queueFamilyIdx);
        VkPhysicalDevice virtual getPhysicalDevice();
        VkPhysicalDeviceFeatures virtual getDeviceFeatures();
        VkPhysicalDeviceProperties virtual getDeviceProperties();
        std::optional<uint32_t> virtual getPhyQueueGraphicsFamilyIndice();
        std::optional<uint32_t> virtual getPhyQueuePresentFamilyIndice();
        const std::vector<const char*>* const getRequiredDevSupportedExts() const;
        bool virtual destroyPhyDevice(VkInstance* instance);
    };
}