#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlSwapChainReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "./AnthemImageContainer.h"

namespace Anthem::Core {
    class AnthemDepthBuffer :
        public virtual AnthemImageContainer,
        public virtual Util::AnthemUtlSwapChainReqBase {
    private:
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        bool ownsSampler = false;
        bool enableStencil = false;
    public:
        AnthemDepthBuffer();
        bool createDepthBuffer(bool useStencil);
        bool createDepthBufferWithSampler(uint32_t height, uint32_t width);
        bool destroyDepthBuffer();
        bool enableMsaa();
        bool enableCubic();

        uint32_t getLayers() const;
        VkFormat getDepthFormat() const;
        bool isStencilEnabled() const;
    };
};
