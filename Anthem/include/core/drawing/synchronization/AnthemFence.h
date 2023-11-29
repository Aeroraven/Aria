#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"

namespace Anthem::Core{
    class AnthemFence:public virtual Util::AnthemUtlLogicalDeviceReqBase{
    private:
        VkFence fence = nullptr;
    public:
        bool createFence();
        const VkFence* getFence() const;
        bool resetFence();
        bool waitForFence();
        bool destroyFence();
    };
}