#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"

namespace Anthem::Core{
    class AnthemSemaphore:public virtual Util::AnthemUtlLogicalDeviceReqBase{
    private:
        VkSemaphore semaphore;
    public:
        bool createSemaphore();
        const VkSemaphore* getSemaphore() const;
    };
}