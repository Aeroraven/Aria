#pragma once
#include "../../base/AnthemBaseImports.h"

namespace Anthem::Core{
    struct AnthemGeneralBufferProp{
        VkBuffer buffer;
        VkBufferCreateInfo bufferCreateInfo = {};
        VkDeviceMemory bufferMem;
        void* mappedMem;
    };
}
