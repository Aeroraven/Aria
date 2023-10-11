#include "../../include/core/AnthemPhyDevice.h"

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
    VkPhysicalDeviceProperties AnthemPhyDevice::getDeviceProperties(){
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
    std::optional<uint32_t> AnthemPhyDevice::getPhyQueueGraphicsFamilyIndice(){
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->queueFamilyIdx.graphicsFamily;
    }
    std::optional<uint32_t> AnthemPhyDevice::getPhyQueuePresentFamilyIndice(){
        ANTH_ASSERT(this->physicalDevice != VK_NULL_HANDLE,"Physical device not specified");
        return this->queueFamilyIdx.presentFamily;
    }
    const std::vector<const char*>* const AnthemPhyDevice::getRequiredDevSupportedExts() const{
        return &this->requiredDeviceSupportedExtension;
    }
    bool AnthemPhyDevice::destroyPhyDevice(VkInstance* instance){
        return true;
    }
    
}