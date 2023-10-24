#include "../../../include/core/drawing/AnthemFramebuffer.h"

namespace Anthem::Core{
    bool AnthemFramebuffer::specifyLogicalDevice(const AnthemLogicalDevice* device){
        this->logicalDevice = device;
        return true;
    }
    bool AnthemFramebuffer::createFromColorAttachment(const AnthemImage* colorImage, const AnthemRenderPass* renderPass){
        ANTH_ASSERT(this->depthBuffer != nullptr,"Depth buffer not specified");
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        this->ctRenderPass = renderPass;
        this->colorAttachment = colorImage;
        VkImageView attachments[] = {
            *colorImage->getImageView(),
            *(this->depthBuffer->getImageView())
        };
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *(renderPass->getRenderPass());
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = colorImage->getWidth();
        framebufferInfo.height = colorImage->getHeight();
        framebufferInfo.layers = 1;
        if(vkCreateFramebuffer(this->logicalDevice->getLogicalDevice(),&framebufferInfo,nullptr,&this->framebuffer) != VK_SUCCESS){
            ANTH_LOGE("Failed to create framebuffer");
            return false;
        }
        ANTH_LOGI("Framebuffer created, ", "Width=",framebufferInfo.width," Height=",framebufferInfo.height);
        return true;
    }
    bool AnthemFramebuffer::createFromSwapchainImageView(VkImageView* swapChainImageView, const AnthemRenderPass* renderPass, int height, int width){
        ANTH_ASSERT(this->depthBuffer != nullptr,"Depth buffer not specified");
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        
        VkImageView attachments[] = {
            *swapChainImageView,
            *(this->depthBuffer->getImageView())
        };
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *(renderPass->getRenderPass());
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;
        if(vkCreateFramebuffer(this->logicalDevice->getLogicalDevice(),&framebufferInfo,nullptr,&this->framebuffer) != VK_SUCCESS){
            ANTH_LOGE("Failed to create framebuffer");
            return false;
        }
        ANTH_LOGI("Framebuffer created, ", "Width=",framebufferInfo.width," Height=",framebufferInfo.height);
        return true;
    }
    bool AnthemFramebuffer::recreateFramebuffer(){
        return this->createFromColorAttachment(this->colorAttachment,this->ctRenderPass);
    }
    bool AnthemFramebuffer::destroyFramebuffers(){
        ANTH_LOGI("Destroying framebuffers");
        vkDestroyFramebuffer(this->logicalDevice->getLogicalDevice(),framebuffer,nullptr);
        return true;
    }
    bool AnthemFramebuffer::setDepthBuffer(AnthemDepthBuffer* depthBuffer){
        this->depthBuffer = depthBuffer;
        return true;
    }
    const VkFramebuffer* AnthemFramebuffer::getFramebuffer() const{
        return &(this->framebuffer);
    }
}