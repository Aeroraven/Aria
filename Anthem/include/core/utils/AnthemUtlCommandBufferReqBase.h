#pragma once
#include "../drawing/AnthemCommandBuffers.h"

namespace Anthem::Core::Util{
    
    class AnthemUtlCommandBufferReqBase{
    protected:
        AnthemCommandBuffers* cmdBufs = nullptr;
    public:
        bool virtual specifyCommandBuffers(AnthemCommandBuffers* cmdBufs);
        const AnthemCommandBuffers* getCommandBuffer() const;
    };
}