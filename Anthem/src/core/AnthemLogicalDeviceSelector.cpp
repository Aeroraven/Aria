#include "../../include/core/AnthemLogicalDeviceSelector.h"

namespace Anthem::Core{
    AnthemLogicalDeviceSelector::AnthemLogicalDeviceSelector(ANTH_SHARED_PTR(AnthemPhyDeviceSelector) phyDeviceSelector){
        this->phyDeviceSelector = phyDeviceSelector;
    }
    bool AnthemLogicalDeviceSelector::createLogicalDevice(){
        //Create Queue
        ANTH_LOGI("Registering Queues");
        registerQueueCreateInfo(phyDeviceSelector->getPhyDeviceQueueFamilyInfo()->graphicsFamily.value());
        registerQueueCreateInfo(phyDeviceSelector->getPhyDeviceQueueFamilyInfo()->presentFamily.value());
        

        //Get Device Extension
        const auto deviceSupportExtensions = this->phyDeviceSelector->getRequiredDevSupportedExts();

        //Get Device Features
        this->phyDeviceSelector->getDeviceFeature(this->creatingFeats);

        //Create Device
        ANTH_LOGI("Creating Queue");
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = this->queueCreateInfo.data();
        createInfo.queueCreateInfoCount = this->queueCreateInfo.size();
        createInfo.enabledExtensionCount = deviceSupportExtensions->size();
        createInfo.ppEnabledExtensionNames = deviceSupportExtensions->data();
        createInfo.enabledLayerCount = 0;
        createInfo.pEnabledFeatures = &(this->creatingFeats);

        if(vkCreateDevice(phyDeviceSelector->getPhyDevice(), &createInfo, nullptr, &logicalDevice)!=VK_SUCCESS){
            ANTH_LOGI("Failed to create logical device");
            return false;
        }
        ANTH_LOGI("Logical device created");
        //Retrieving Queue Handlers
        this->retrieveQueues();
        return true;
    }
    bool AnthemLogicalDeviceSelector::destroyLogicalDevice(){
        ANTH_LOGI("Destroying logical device");
        vkDestroyDevice(logicalDevice, nullptr);
        return true;
    }
    bool AnthemLogicalDeviceSelector::retrieveQueues(){
        vkGetDeviceQueue(logicalDevice, phyDeviceSelector->getPhyDeviceQueueFamilyInfo()->graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, phyDeviceSelector->getPhyDeviceQueueFamilyInfo()->presentFamily.value(), 0, &presentationQueue);
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
    
}   