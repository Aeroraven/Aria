#pragma once
#include "AnthemDefs.h"
#include "AnthemPhyDeviceSelector.h"


namespace Anthem::Core{
    class AnthemLogicalDeviceSelector{
    private:
        ANTH_SHARED_PTR(AnthemPhyDeviceSelector) phyDeviceSelector;
        VkDevice logicalDevice = VK_NULL_HANDLE;
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        VkDeviceCreateInfo createInfo = {};
        VkQueue graphicsQueue;
    public:
        AnthemLogicalDeviceSelector(ANTH_SHARED_PTR(AnthemPhyDeviceSelector) phyDeviceSelector);
        bool virtual createLogicalDevice();
        bool virtual destroyLogicalDevice();
        bool virtual retrieveGraphicsQueue();
    };
}