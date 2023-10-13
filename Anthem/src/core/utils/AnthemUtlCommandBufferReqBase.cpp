#include "../../../include/core/utils/AnthemUtlCommandBufferReqBase.h"

namespace Anthem::Core::Util{
    bool AnthemUtlCommandBufferReqBase::specifyCommandBuffers(const AnthemCommandBuffers* cmdBuf){
        this->cmdBufs = cmdBuf;
        return true;
    }
    const AnthemCommandBuffers* AnthemUtlCommandBufferReqBase::getCommandBuffer() const{
        return this->cmdBufs;
    }
}