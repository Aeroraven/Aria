#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../utils/AnthemUtlSwapChainReqBase.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"
#include "./AnthemImageContainer.h"

namespace Anthem::Core{
    class AnthemDepthBuffer:
    public virtual AnthemImageContainer,
    public virtual Util::AnthemUtlSwapChainReqBase{
    private:
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        bool ownsSampler = false;
    public:
        AnthemDepthBuffer(){
            this->image.desiredLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        }
        bool createDepthBuffer();
        bool createDepthBufferWithSampler(uint32_t height, uint32_t width);
        bool destroyDepthBuffer();
        bool enableMsaa();
        bool enableCubic();

        uint32_t getLayers() const;
        VkFormat getDepthFormat() const{
            return depthFormat;
        }
    };
}