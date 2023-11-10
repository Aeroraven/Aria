#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"

namespace Anthem::Core{
    class AnthemSemaphore:public virtual Util::AnthemUtlLogicalDeviceReqBase{
    private:
        VkSemaphore semaphore = nullptr;
    public:
        bool createSemaphore();
        const VkSemaphore* getSemaphore() const;
        bool destroySemaphore();
    };
}