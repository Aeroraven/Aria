#include "../../../../include/core/drawing/image/AnthemDepthBuffer.h"

namespace Anthem::Core{
    AnthemDepthBuffer::AnthemDepthBuffer() {
        this->image.desiredLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }
    bool AnthemDepthBuffer::createDepthBuffer(bool useStencil){
        this->enableStencil = useStencil;
        if (useStencil) {
            this->depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        }
        this->createImageInternal(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            this->depthFormat,this->swapChain->getSwapChainExtentWidth(),this->swapChain->getSwapChainExtentHeight(),1);
        auto aspectFlag = useStencil ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_DEPTH_BIT;
        this->createImageViewInternal(aspectFlag);
        this->createImageTransitionLayoutInternal(VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,aspectFlag);
        return true;
    }
    bool AnthemDepthBuffer::createDepthBufferWithSampler(uint32_t height,uint32_t width){
        if (height == 0 || width == 0) {
            height = this->swapChain->getSwapChainExtentHeight();
            width = this->swapChain->getSwapChainExtentWidth();
        }
        this->createImageInternal(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            this->depthFormat,width,height,1);
        this->createImageViewInternal(VK_IMAGE_ASPECT_DEPTH_BIT);
        this->createSampler();
        ownsSampler = true;
        return true;
    }
    bool AnthemDepthBuffer::destroyDepthBuffer(){
        if(this->ownsSampler){
            vkDestroySampler(this->logicalDevice->getLogicalDevice(),this->sampler,nullptr);
        }
        this->destroyImageViewInternal();
        this->destroyImageInternal();
        return true;
    }
    bool AnthemDepthBuffer::enableMsaa(){
        this->image.msaaCount = this->phyDevice->getMaxSampleCount();
        return true;
    }
    bool AnthemDepthBuffer::enableCubic() {
        this->image.extraFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        this->image.isCubic = true;
        this->image.layerCounts = 6;
        return true;
    }
    uint32_t AnthemDepthBuffer::getLayers() const {
        return this->image.layerCounts;
    }
    VkFormat AnthemDepthBuffer::getDepthFormat() const {
        return depthFormat;
    }
    bool AnthemDepthBuffer::isStencilEnabled() const {
        return enableStencil;
    }
}