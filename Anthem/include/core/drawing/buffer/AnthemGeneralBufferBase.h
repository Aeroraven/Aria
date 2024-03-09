#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "./../buffer/AnthemGeneralBufferProp.h"
#include "./../AnthemCommandBuffers.h"

namespace Anthem::Core{

    struct AnthemBufferBarrierProp {
        VkAccessFlags access;
        VkPipelineStageFlags stage;
        uint32_t queueFamily;
    };

    class AnthemGeneralBufferBase:public virtual Util::AnthemUtlLogicalDeviceReqBase,
    public virtual Util::AnthemUtlPhyDeviceReqBase{
    protected:
        uint32_t virtual calculateBufferSize() = 0;
        bool virtual createBufferInternal(AnthemGeneralBufferProp* bufProp, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp);
        bool virtual bindBufferInternal(AnthemGeneralBufferProp* bufProp);
        bool virtual setupBufferBarrierInternal(VkCommandBuffer cmdBuf,AnthemGeneralBufferProp* bufProp, AnthemBufferBarrierProp* src, AnthemBufferBarrierProp* dst);
        
    };
}