#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "./../buffer/AnthemGeneralBufferProp.h"

namespace Anthem::Core{

    class AnthemGeneralBufferBase:public virtual Util::AnthemUtlLogicalDeviceReqBase,
    public virtual Util::AnthemUtlPhyDeviceReqBase{
    protected:
        uint32_t virtual calculateBufferSize() = 0;
        bool virtual createBufferInternal(AnthemGeneralBufferProp* bufProp, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memProp);
        bool virtual bindBufferInternal(AnthemGeneralBufferProp* bufProp);
        
    };
}