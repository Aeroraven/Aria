#pragma once
#include "../drawing/AnthemCommandBuffers.h"

namespace Anthem::Core::Util{
    class AnthemUtlCommandBufferReqBase{
    protected:
        const AnthemCommandBuffers* cmdBufs = nullptr;
    public:
        bool virtual specifyCommandBuffers(const AnthemCommandBuffers* cmdBufs);
        const AnthemCommandBuffers* getCommandBuffer() const;
    };
}