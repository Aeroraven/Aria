#pragma once
#include "AnthemDefs.h"
#include "AnthemLogger.h"

namespace Anthem::Core{
    struct AnthemPhyQueueFamilyIdx{
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> computeFamily;
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
        std::vector<const char*> requiredDeviceSupportedExtension = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
        };
        
    public:
        AnthemPhyDevice(); 
        bool virtual specifyDevice(VkPhysicalDevice device,AnthemPhyQueueFamilyIdx* queueFamilyIdx);
        VkPhysicalDevice virtual getPhysicalDevice() const;
        VkPhysicalDeviceFeatures virtual getDeviceFeatures();
        VkPhysicalDeviceProperties virtual getDeviceProperties() const;
        std::optional<uint32_t> virtual getPhyQueueGraphicsFamilyIndice() const;
        std::optional<uint32_t> virtual getPhyQueueComputeFamilyIndice() const;
        std::optional<uint32_t> virtual getPhyQueuePresentFamilyIndice() const;
        const std::vector<const char*>* const getRequiredDevSupportedExts() const;
        bool virtual destroyPhyDevice(VkInstance* instance);
        uint32_t virtual findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        VkSampleCountFlags getMaxSampleCount() const;
    };
}