#pragma once
#include "../../base/AnthemLogicalDevice.h"
#include "../../base/AnthemPhyDevice.h"
#include "AnthemGeneralBufferProp.h"

namespace Anthem::Core{
    class AnthemGeneralBufferUtilBase{
    public:
        uint32_t virtual calculateBufferSize() { return 0; };
        bool virtual createBufferInternalUt(const AnthemLogicalDevice* logicalDevice,const AnthemPhyDevice* phyDevice,
            AnthemGeneralBufferProp* bufProp, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp,
            void* memAllocFlags = nullptr, uint32_t size = 0);
        VkDeviceAddress virtual getBufferDeviceAddressUt(const AnthemLogicalDevice* logicalDevice, AnthemGeneralBufferProp* bufProp) const;
        bool virtual bindBufferInternalUt(const AnthemLogicalDevice* logicalDevice, AnthemGeneralBufferProp* bufProp);
        bool virtual copyDataToBufferInternalUt(const AnthemLogicalDevice* logicalDevice,AnthemGeneralBufferProp* bufProp, void* data, uint32_t size, bool flush) const;
    };
}