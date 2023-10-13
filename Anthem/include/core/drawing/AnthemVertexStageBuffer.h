#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../utils/AnthemUtlCommandBufferReqBase.h"


namespace Anthem::Core{
    struct AnthemVertexStageBufferProp{
        VkBuffer buffer;
        VkBufferCreateInfo bufferCreateInfo = {};
        VkDeviceMemory bufferMem;
    };
    class AnthemVertexStageBuffer: public Util::AnthemUtlLogicalDeviceReqBase, 
        public Util::AnthemUtlPhyDeviceReqBase, public Util::AnthemUtlCommandBufferReqBase{
    protected:
        AnthemVertexStageBufferProp dstBuffer;
        AnthemVertexStageBufferProp stagingBuffer;
        
    protected:
        uint32_t virtual calculateBufferSize() = 0;
        void virtual getRawBufferData(void** dataDst) = 0;
    public:
        bool virtual createBuffer() = 0;
        
    protected:
        bool virtual bindBufferInternal(AnthemVertexStageBufferProp* bufProp);
        bool virtual copyStagingToVertexBuffer();
        bool virtual createBufferInternal(AnthemVertexStageBufferProp* bufProp, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memProp);
        
    public:
        bool virtual destroyBuffer();
        const VkBuffer* getDestBufferObject() const{
            return &(this->dstBuffer.buffer);
        }
    };
}