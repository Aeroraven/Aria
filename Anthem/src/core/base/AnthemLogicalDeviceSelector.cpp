#include "../../../include/core/base/AnthemLogicalDeviceSelector.h"

namespace Anthem::Core{
    AnthemLogicalDeviceSelector::AnthemLogicalDeviceSelector(AnthemPhyDevice* phyDevice){
        this->phyDevice = phyDevice;
    }
    bool AnthemLogicalDeviceSelector::createLogicalDevice(){
        //Create Queue
        ANTH_LOGI("Registering Queues");
        registerQueueCreateInfo(phyDevice->getPhyQueueGraphicsFamilyIndice().value());
        registerQueueCreateInfo(phyDevice->getPhyQueuePresentFamilyIndice().value());
        

        //Get Device Extension
        const auto deviceSupportExtensions = phyDevice->getRequiredDevSupportedExts();

        //Get Device Features
        this->creatingFeats = phyDevice->getDeviceFeatures();

        //Create Device
        ANTH_LOGI("Creating Queue");
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = this->queueCreateInfo.data();
        createInfo.queueCreateInfoCount = this->queueCreateInfo.size();
        createInfo.enabledExtensionCount = deviceSupportExtensions->size();
        createInfo.ppEnabledExtensionNames = deviceSupportExtensions->data();
        createInfo.enabledLayerCount = 0;
        createInfo.pEnabledFeatures = &(this->creatingFeats);

        if(vkCreateDevice(phyDevice->getPhysicalDevice(), &createInfo, nullptr, &logicalDevice)!=VK_SUCCESS){
            ANTH_LOGI("Failed to create logical device");
            return false;
        }
        ANTH_LOGI("Logical device created");
        //Retrieving Queue Handlers
        this->retrieveQueues();
        return true;
    }
    bool AnthemLogicalDeviceSelector::destroyLogicalDevice(){
        ANTH_DEPRECATED_MSG;
        ANTH_LOGI("Destroying logical device");
        vkDestroyDevice(logicalDevice, nullptr);
        return true;
    }
    bool AnthemLogicalDeviceSelector::retrieveQueues(){
        vkGetDeviceQueue(logicalDevice, phyDevice->getPhyQueueGraphicsFamilyIndice().value(), 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, phyDevice->getPhyQueuePresentFamilyIndice().value(), 0, &presentationQueue);
        return true;
    }
    bool AnthemLogicalDeviceSelector::registerQueueCreateInfo(std::optional<uint32_t> familyIdx,float priority){
        if(!familyIdx.has_value()){
            return false;
        }
        if(queueExistingId.count(familyIdx.value())!=0){
            return false;
        }
        VkDeviceQueueCreateInfo qci;
        qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci.queueFamilyIndex = familyIdx.value();
        qci.queueCount = 1;
        auto queuePriority = ANTH_MAKE_UNSAFE(float)(1.0f);
        qci.pQueuePriorities = queuePriority;
        qci.pNext = nullptr;
        qci.flags = 0;
        this->queueCreateInfo.push_back(qci);
        queueExistingId.insert(familyIdx.value());
        return true;
    }
    VkDevice AnthemLogicalDeviceSelector::getLogicalDevice(){
        return this->logicalDevice;
    }
    bool AnthemLogicalDeviceSelector::getLogicalDevice(AnthemLogicalDevice* logicalDevice){
        logicalDevice->specifyDevice(this->logicalDevice);
        return true;
    }
}   