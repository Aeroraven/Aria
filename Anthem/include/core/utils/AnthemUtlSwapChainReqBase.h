#pragma once
#include "../base/AnthemSwapChain.h"

namespace Anthem::Core::Util{
    class AnthemUtlSwapChainReqBase{
    protected:
        const AnthemSwapChain* swapChain = nullptr;
    public:
        bool virtual specifySwapChain(const AnthemSwapChain* target);
        const AnthemSwapChain* getSwapChain() const;
    };
}