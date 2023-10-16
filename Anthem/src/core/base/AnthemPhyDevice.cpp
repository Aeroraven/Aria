#include "../../../include/core/base/AnthemPhyDevice.h"

namespace Anthem::Core{
    AnthemPhyDevice::AnthemPhyDevice(){
        
    }
    VkPhysicalDevice AnthemPhyDevice::getPhysicalDevice(){
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
    bool AnthemPhyDevice::specifyDevice(VkPhysicalDevice device,AnthemPhyQueueFamilyIdx* queueFamilyIdx){
        this->physicalDevice = device;
        vkGetPhysicalDeviceFeatures(device,&this->deviceFeatures);
        vkGetPhysicalDeviceProperties(device,&this->deviceProperties);
        this->queueFamilyIdx.graphicsFamily = queueFamilyIdx->graphicsFamily;
        this->queueFamilyIdx.presentFamily = queueFamilyIdx->presentFamily;

        return true;
    }
    std::optional<uint32_t> AnthemPhyDevice::getPhyQueueGraphicsFamilyIndice() const{
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->queueFamilyIdx.graphicsFamily;
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
                ANTH_LOGI("Found suitable memory type, idx=",i);
                return i;
            }
        }
        ANTH_LOGE("Failed to find suitable memory type!");
        return 0;
    }
}