#include "../../../include/core/base/AnthemLogicalDevice.h"

namespace Anthem::Core{
    AnthemLogicalDevice::AnthemLogicalDevice(){
        
    }
    bool AnthemLogicalDevice::specifyDevice(VkDevice device){
        this->logicalDevice = device;
        return true;
    }
    VkDevice AnthemLogicalDevice::getLogicalDevice(){
        ANTH_ASSERT(this->logicalDevice != VK_NULL_HANDLE,"Logical device not specified");
        return this->logicalDevice;
    }
    bool AnthemLogicalDevice::destroyLogicalDevice(VkInstance* instance){
        vkDestroyDevice(this->logicalDevice,nullptr);
        this->specifiedDevice = false;
        return true;
    }
    
}