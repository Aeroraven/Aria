#pragma once
#include "../../base/AnthemLogicalDevice.h"
#include "../../base/AnthemPhyDevice.h"
#include "AnthemGeneralBufferProp.h"

namespace Anthem::Core{
    class AnthemGeneralBufferUtilBase{
    public:
        uint32_t virtual calculateBufferSize() = 0;
        bool virtual createBufferInternalUt(const AnthemLogicalDevice* logicalDevice,const AnthemPhyDevice* phyDevice, AnthemGeneralBufferProp* bufProp, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memProp);
        bool virtual bindBufferInternalUt(const AnthemLogicalDevice* logicalDevice, AnthemGeneralBufferProp* bufProp);
    };
}