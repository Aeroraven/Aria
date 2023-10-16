#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../utils/AnthemUtlCommandBufferReqBase.h"


namespace Anthem::Core{
    struct AnthemGeneralBufferProp{
        VkBuffer buffer;
        VkBufferCreateInfo bufferCreateInfo = {};
        VkDeviceMemory bufferMem;
    };

    class AnthemGeneralBufferBase:public Util::AnthemUtlLogicalDeviceReqBase,
    public Util::AnthemUtlPhyDeviceReqBase{
    protected:
        uint32_t virtual calculateBufferSize() = 0;
        bool virtual createBufferInternal(AnthemGeneralBufferProp* bufProp, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memProp);
        bool virtual bindBufferInternal(AnthemGeneralBufferProp* bufProp);
        
    };
}