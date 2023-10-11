#pragma once
#include "AnthemDefs.h"
#include "AnthemPhyDeviceSelector.h"
#include "AnthemPhyDevice.h"
#include "AnthemLogicalDevice.h"

namespace Anthem::Core{
    class AnthemLogicalDeviceSelector{
    private:
        AnthemPhyDevice* phyDevice;
        VkDevice logicalDevice = VK_NULL_HANDLE;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfo = {};
        std::set<uint32_t> queueExistingId = {};
        VkDeviceCreateInfo createInfo = {};
        VkQueue graphicsQueue;
        VkQueue presentationQueue;
        VkPhysicalDeviceFeatures creatingFeats;
    public:
        AnthemLogicalDeviceSelector(AnthemPhyDevice* phyDevice);
        bool virtual registerQueueCreateInfo(std::optional<uint32_t> familyIdx,float priority=1.0);
        bool virtual createLogicalDevice();
        bool virtual destroyLogicalDevice();
        bool virtual retrieveQueues();
        VkDevice virtual getLogicalDevice();
        bool virtual getLogicalDevice(AnthemLogicalDevice* logicalDevice);
    };
}