#pragma once
#include "AnthemLogger.h"
#include "AnthemDefs.h"

namespace Anthem::Core{
    class AnthemLogicalDevice{
    private:
        VkDevice logicalDevice = VK_NULL_HANDLE;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        bool specifiedDevice = false;
    public:
        AnthemLogicalDevice();
        bool virtual specifyDevice(VkDevice device);
        bool virtual specifyGraphicsQueue(VkQueue queue);
        bool virtual specifyPresentQueue(VkQueue queue);

        VkDevice virtual getLogicalDevice() const;
        VkQueue virtual getGraphicsQueue() const;
        VkQueue virtual getPresentQueue() const;

        bool virtual destroyLogicalDevice(VkInstance* instance);
        bool virtual waitForIdle() const;
    };

}