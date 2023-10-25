#include "../../../../include/core/drawing/image/AnthemDepthBuffer.h"

namespace Anthem::Core{
    bool AnthemDepthBuffer::createDepthBuffer(){
        this->createImageInternal(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,this->depthFormat,this->swapChain->getSwapChainExtentWidth(),this->swapChain->getSwapChainExtentHeight());
        this->createImageViewInternal(VK_IMAGE_ASPECT_DEPTH_BIT);
        this->createImageTransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        return true;
    }
    bool AnthemDepthBuffer::destroyDepthBuffer(){
        this->destroyImageViewInternal();
        this->destroyImageInternal();
        return true;
    }
    bool AnthemDepthBuffer::enableMsaa(){
        this->image.msaaCount = this->phyDevice->getMaxSampleCount();
        return true;
    }
}