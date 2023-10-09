#include "../../include/core/AnthemLogicalDeviceSelector.h"

namespace Anthem::Core{
    AnthemLogicalDeviceSelector::AnthemLogicalDeviceSelector(ANTH_SHARED_PTR(AnthemPhyDeviceSelector) phyDeviceSelector){
        this->phyDeviceSelector = phyDeviceSelector;
    }
    bool AnthemLogicalDeviceSelector::createLogicalDevice(){
        //Create Queue
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = phyDeviceSelector->getPhyDeviceQueueFamilyInfo()->graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        auto queuePriority = ANTH_MAKE_UNSAFE(float)(1.0f);
        queueCreateInfo.pQueuePriorities = queuePriority;

        //Create Device
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.enabledExtensionCount = 0;
        createInfo.enabledLayerCount = 0;

        if(vkCreateDevice(phyDeviceSelector->getPhyDevice(), &createInfo, nullptr, &logicalDevice)!=VK_SUCCESS){
            ANTH_LOGI("Failed to create logical device");
            return false;
        }
        ANTH_LOGI("Logical device created");
        //Retrieving Queue Handlers
        this->retrieveGraphicsQueue();
        return true;
    }
    bool AnthemLogicalDeviceSelector::destroyLogicalDevice(){
        ANTH_LOGI("Destroying logical device");
        vkDestroyDevice(logicalDevice, nullptr);
        return true;
    }
    bool AnthemLogicalDeviceSelector::retrieveGraphicsQueue(){
        vkGetDeviceQueue(logicalDevice, phyDeviceSelector->getPhyDeviceQueueFamilyInfo()->graphicsFamily.value(), 0, &graphicsQueue);
        return true;
    }
}   