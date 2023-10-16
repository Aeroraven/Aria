#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../utils/AnthemUtlCommandBufferReqBase.h"
#include "AnthemGeneralBufferBase.h"


namespace Anthem::Core{
    struct AnthemVertexStageBufferProp:public AnthemGeneralBufferProp{

    };
    class AnthemVertexStageBuffer: public AnthemGeneralBufferBase, public Util::AnthemUtlCommandBufferReqBase{
    protected:
        AnthemVertexStageBufferProp dstBuffer;
        AnthemVertexStageBufferProp stagingBuffer;
        
    protected:
        
        void virtual getRawBufferData(void** dataDst) = 0;
    public:
        bool virtual createBuffer() = 0;
        
    protected:
        bool virtual copyStagingToVertexBuffer();
        
    public:
        bool virtual destroyBuffer();
        const VkBuffer* getDestBufferObject() const{
            return &(this->dstBuffer.buffer);
        }
    };
}