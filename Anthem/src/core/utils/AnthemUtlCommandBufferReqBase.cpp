#include "../../../include/core/utils/AnthemUtlCommandBufferReqBase.h"

namespace Anthem::Core::Util{
    using namespace Anthem::Core;
    bool AnthemUtlCommandBufferReqBase::specifyCommandBuffers(AnthemCommandBuffers* cmdBuf){
        this->cmdBufs = cmdBuf;
        return true;
    }
    const AnthemCommandBuffers* AnthemUtlCommandBufferReqBase::getCommandBuffer() const{
        return this->cmdBufs;
    }
}