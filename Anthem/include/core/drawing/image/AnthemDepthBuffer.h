#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlSwapChainReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "./AnthemImageContainer.h"

namespace Anthem::Core{
    class AnthemDepthBuffer:
    public AnthemImageContainer,
    public Util::AnthemUtlSwapChainReqBase{
    private:
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    public:
        bool createDepthBuffer();
        bool destroyDepthBuffer();
        VkFormat getDepthFormat() const{
            return depthFormat;
        }
    };
}