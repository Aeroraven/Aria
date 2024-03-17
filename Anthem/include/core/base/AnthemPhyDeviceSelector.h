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
        std::vector<const char*> requiredDeviceSupportedExtension = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
            //Mesh Shader
            VK_EXT_MESH_SHADER_EXTENSION_NAME,

#ifdef AT_FEATURE_RAYTRACING_ENABLED
            //Ray Tracing
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
#endif
        };
        
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
