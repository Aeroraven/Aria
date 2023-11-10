#pragma once
#include "../../base/AnthemBaseImports.h"

namespace Anthem::Core{
    struct AnthemGeneralBufferProp{
        VkBuffer buffer = nullptr;
        VkBufferCreateInfo bufferCreateInfo = {};
        VkDeviceMemory bufferMem = nullptr;
        void* mappedMem = nullptr;
    };
}
