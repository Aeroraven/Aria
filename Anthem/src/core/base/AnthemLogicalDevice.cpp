#include "../../../include/core/base/AnthemLogicalDevice.h"

namespace Anthem::Core{
    AnthemLogicalDevice::AnthemLogicalDevice(){
        
    }
    bool AnthemLogicalDevice::specifyDevice(VkDevice device){
        this->logicalDevice = device;
        return true;
    }
    VkDevice AnthemLogicalDevice::getLogicalDevice() const{
        ANTH_ASSERT(this->logicalDevice != VK_NULL_HANDLE,"Logical device not specified");
        return this->logicalDevice;
    }
    bool AnthemLogicalDevice::destroyLogicalDevice(const VkInstance* instance){
        vkDestroyDevice(this->logicalDevice,nullptr);
        this->specifiedDevice = false;
        return true;
    }
    bool AnthemLogicalDevice::specifyGraphicsQueue(VkQueue queue){
        this->graphicsQueue = queue;
        return true;
    }
    bool AnthemLogicalDevice::specifyComputeQueue(VkQueue queue) {
        this->computeQueue = queue;
        return true;
    }
    VkQueue AnthemLogicalDevice::getGraphicsQueue() const{
        ANTH_ASSERT(this->graphicsQueue != VK_NULL_HANDLE,"Graphics queue not specified");
        return this->graphicsQueue;
    }
    bool AnthemLogicalDevice::specifyPresentQueue(VkQueue queue){
        this->presentQueue = queue;
        return true;
    }
    VkQueue AnthemLogicalDevice::getPresentQueue() const{
        ANTH_ASSERT(this->presentQueue != VK_NULL_HANDLE,"Present queue not specified");
        return this->presentQueue;
    }
    VkQueue AnthemLogicalDevice::getComputeQueue() const {
        ANTH_ASSERT(this->computeQueue != VK_NULL_HANDLE, "Present queue not specified");
        return this->computeQueue;
    }
    bool AnthemLogicalDevice::waitForIdle() const{
        vkDeviceWaitIdle(this->logicalDevice);
        return true;
    }

}