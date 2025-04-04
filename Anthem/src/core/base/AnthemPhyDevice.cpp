#include "../../../include/core/base/AnthemPhyDevice.h"

namespace Anthem::Core{
    AnthemPhyDevice::AnthemPhyDevice(){
        
    }
    VkPhysicalDevice AnthemPhyDevice::getPhysicalDevice() const{
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->physicalDevice;
    }
    VkPhysicalDeviceFeatures AnthemPhyDevice::getDeviceFeatures(){
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->deviceFeatures;
    }
    VkPhysicalDeviceProperties AnthemPhyDevice::getDeviceProperties() const{
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->deviceProperties;
    }
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR AnthemPhyDevice::getDeivceRaytracingProperties() const{
        return this->rtPipelineProperty;
    }
    bool AnthemPhyDevice::specifyDevice(VkPhysicalDevice device,AnthemPhyQueueFamilyIdx* queueFamilyIdx){
        this->physicalDevice = device;
        vkGetPhysicalDeviceFeatures(device,&this->deviceFeatures);
        vkGetPhysicalDeviceProperties(device,&this->deviceProperties);

#ifdef AT_FEATURE_RAYTRACING_ENABLED
        rtPipelineProperty.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &rtPipelineProperty;
        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);
#endif

        this->queueFamilyIdx.graphicsFamily = queueFamilyIdx->graphicsFamily;
        this->queueFamilyIdx.presentFamily = queueFamilyIdx->presentFamily;
        this->queueFamilyIdx.computeFamily = queueFamilyIdx->computeFamily;
        return true;
    }
    std::optional<uint32_t> AnthemPhyDevice::getPhyQueueGraphicsFamilyIndice() const{
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->queueFamilyIdx.graphicsFamily;
    }
    std::optional<uint32_t> AnthemPhyDevice::getPhyQueueComputeFamilyIndice() const{
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->queueFamilyIdx.computeFamily;
    }
    std::optional<uint32_t> AnthemPhyDevice::getPhyQueuePresentFamilyIndice() const{
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->queueFamilyIdx.presentFamily;
    }
    const std::vector<const char*>* const AnthemPhyDevice::getRequiredDevSupportedExts() const{
        return &this->requiredDeviceSupportedExtension;
    }
    bool AnthemPhyDevice::destroyPhyDevice(VkInstance* instance){
        return true;
    }
    uint32_t AnthemPhyDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const{
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        ANTH_LOGE("Failed to find suitable memory type!");
        return 0;
    }
    VkSampleCountFlags AnthemPhyDevice::getMaxSampleCount() const{
        auto& property = this->deviceProperties;
        VkSampleCountFlags flags = property.limits.framebufferDepthSampleCounts & property.limits.sampledImageDepthSampleCounts;
        for(int i=64;i>=1;(i>>=1)){
            if(flags & i){
                return i;
            }
        }
        ANTH_LOGE("Failed to find available count");
        return 0;
    }
}