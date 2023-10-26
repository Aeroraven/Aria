#include "../../../include/core/drawing/AnthemSwapchainFramebuffer.h"

namespace Anthem::Core{
    bool AnthemSwapchainFramebuffer::specifyLogicalDevice(const AnthemLogicalDevice* device){
        this->logicalDevice = device;
        return true;
    }
    bool AnthemSwapchainFramebuffer::createFramebuffersFromSwapChain(const AnthemSwapChain* swapChain,const AnthemRenderPass* renderPass){
        ANTH_ASSERT(this->depthBuffer != nullptr,"Depth buffer not specified");
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        this->framebuffers.resize(swapChain->getSwapChainImageViews()->size());

        for(size_t i = 0; i < swapChain->getSwapChainImageViews()->size(); i++){
            this->framebuffers[i].setDepthBuffer(this->depthBuffer);
            this->framebuffers[i].specifyLogicalDevice(this->logicalDevice);
            auto x = (VkImageView)swapChain->getSwapChainImageViews()->at(i);
            ANTH_LOGI("Creating FB:",i);
            this->framebuffers[i].createFromSwapchainImageView(&x,renderPass,swapChain->getSwapChainExtent()->height,
                swapChain->getSwapChainExtent()->width);
        }
        return true;
    }
    bool AnthemSwapchainFramebuffer::destroyFramebuffers(){
        ANTH_LOGI("Destroying framebuffers");
        for(auto& framebuffer : this->framebuffers){
            framebuffer.destroyFramebuffers();
        }
        this->framebuffers.clear();
        return true;
    }
    bool AnthemSwapchainFramebuffer::setDepthBuffer(AnthemDepthBuffer* depthBuffer){
        this->depthBuffer = depthBuffer;
        return true;
    }
    const AnthemFramebuffer* AnthemSwapchainFramebuffer::getFramebufferObject(uint32_t index) const{
        ANTH_ASSERT(index < this->framebuffers.size(),"Invalid index");
        return &(this->framebuffers[index]);
    }
}