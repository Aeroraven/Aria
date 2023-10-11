#pragma once
#include "AnthemDefs.h"
#include "AnthemPhyDeviceSelector.h"


namespace Anthem::Core{
    class AnthemLogicalDeviceSelector{
    private:
        ANTH_SHARED_PTR(AnthemPhyDeviceSelector) phyDeviceSelector;
        VkDevice logicalDevice = VK_NULL_HANDLE;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfo = {};
        std::set<uint32_t> queueExistingId = {};
        VkDeviceCreateInfo createInfo = {};
        VkQueue graphicsQueue;
        VkQueue presentationQueue;
        VkPhysicalDeviceFeatures creatingFeats;
    public:
        AnthemLogicalDeviceSelector(ANTH_SHARED_PTR(AnthemPhyDeviceSelector) phyDeviceSelector);
        bool virtual registerQueueCreateInfo(std::optional<uint32_t> familyIdx,float priority=1.0);
        bool virtual createLogicalDevice();
        bool virtual destroyLogicalDevice();
        bool virtual retrieveQueues();
    };
}