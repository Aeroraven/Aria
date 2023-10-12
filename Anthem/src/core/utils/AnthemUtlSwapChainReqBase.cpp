#include "../../../include/core/utils/AnthemUtlSwapChainReqBase.h"

namespace Anthem::Core::Util{
    bool AnthemUtlSwapChainReqBase::specifySwapChain(const AnthemSwapChain* target){
        swapChain = target;
        return true;
    }
    const AnthemSwapChain* AnthemUtlSwapChainReqBase::getSwapChain() const{
        return swapChain;
    }
}