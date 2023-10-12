#pragma once
#include "AnthemLogger.h"
#include "AnthemDefs.h"

namespace Anthem::Core{
    class AnthemLogicalDevice{
    private:
        VkDevice logicalDevice = VK_NULL_HANDLE;
        bool specifiedDevice = false;
    public:
        AnthemLogicalDevice();
        bool virtual specifyDevice(VkDevice device);
        VkDevice virtual getLogicalDevice() const;
        bool virtual destroyLogicalDevice(VkInstance* instance);
    };

}