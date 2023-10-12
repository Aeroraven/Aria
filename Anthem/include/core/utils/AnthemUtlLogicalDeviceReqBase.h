#pragma once
#include "../base/AnthemLogicalDevice.h"

namespace Anthem::Core::Util{
    class AnthemUtlLogicalDeviceReqBase{
    protected:
        const AnthemLogicalDevice* logicalDevice = nullptr;
    public:
        bool virtual specifyLogicalDevice(const AnthemLogicalDevice* device);
        const AnthemLogicalDevice* getLogicalDevice() const;
    };
}