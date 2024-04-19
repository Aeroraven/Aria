#include "../../../include/core/drawing/AnthemFramebuffer.h"

namespace Anthem::Core{
    bool AnthemFramebuffer::specifyLogicalDevice(const AnthemLogicalDevice* device){
        this->logicalDevice = device;
        return true;
    }
    bool AnthemFramebuffer::createFromColorAttachment(const std::vector<const IAnthemImageViewContainer*>* colorImages, const AnthemRenderPass* renderPass){
        //ANTH_ASSERT(this->depthBuffer != nullptr,"Depth buffer not specified");
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        this->ctRenderPass = renderPass;
        if(colorImages != &this->colorAttachment){
            this->colorAttachment.clear();
            for(auto p:*colorImages){
                this->colorAttachment.push_back(p);
            }
        }

        std::vector<VkImageView> framebufferAttachment;
        int colorImageCur = 0;
        auto totalRenderPassAttachments = renderPass->getTotalAttachmentCnt();
        auto colorImageWid = this->depthBuffer ? (this->depthBuffer->getImageWidth()) : 0;
        auto colorImageHeight = this->depthBuffer ? (this->depthBuffer->getImageHeight()) : 0;
        auto colorImageLayers = this->depthBuffer ? this->depthBuffer->getLayers() : 0;
        
        if((*colorImages).size()>0){
            colorImageWid = (*colorImages)[0]->getWidth();
            colorImageHeight = (*colorImages)[0]->getHeight();
            colorImageLayers = (*colorImages)[0]->getLayers();
        }

        for(uint32_t i=0;i<totalRenderPassAttachments;i++){
            ANTH_LOGI("Proc Attachment ",i);
            auto attType = renderPass->getAttachmentType(i);
            if(attType == AT_ARPCA_COLOR || attType == AT_ARPCA_COLOR_MSAA){
                auto& tmp = (*colorImages)[colorImageCur++];
                ANTH_LOGI("Fv",tmp->getImageViewFrameBuffer());
                framebufferAttachment.push_back(*(tmp->getImageViewFrameBuffer()));
                ANTH_ASSERT( tmp->getWidth() == colorImageWid, "Incompatible size");
                ANTH_ASSERT( tmp->getHeight() == colorImageHeight, "Incompatible size");
                ANTH_ASSERT( tmp->getLayers() == colorImageLayers, "Incompatible layer size");

            }else if(attType == AT_ARPCA_DEPTH){
                framebufferAttachment.push_back(*(this->depthBuffer->getImageView()));
            }
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *(renderPass->getRenderPass());
        framebufferInfo.attachmentCount = static_cast<uint32_t>(framebufferAttachment.size());
        framebufferInfo.pAttachments = framebufferAttachment.data();
        framebufferInfo.width = colorImageWid;
        framebufferInfo.height = colorImageHeight;
        framebufferInfo.layers = colorImageLayers;
        ANTH_LOGI("Layers:", colorImageLayers);
        if(vkCreateFramebuffer(this->logicalDevice->getLogicalDevice(),&framebufferInfo,nullptr,&this->framebuffer) != VK_SUCCESS){
            ANTH_LOGE("Failed to create framebuffer");
            return false;
        }
        this->framebufferExtent.height = framebufferInfo.height;
        this->framebufferExtent.width = framebufferInfo.width;
        ANTH_LOGI("Framebuffer created, ", "Width=",framebufferInfo.width," Height=",framebufferInfo.height);
        return true;
    }
    bool AnthemFramebuffer::createFromSwapchainImageView(VkImageView* swapChainImageView, const AnthemRenderPass* renderPass, int height, int width){
        ANTH_ASSERT(this->depthBuffer != nullptr,"Depth buffer not specified");
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        
        std::vector<VkImageView> attachments = {};
        attachments.push_back(*swapChainImageView);
        if(renderPass->getSetupOption().msaaType == AT_ARPMT_MSAA){
            attachments.push_back(*renderPass->getSetupOption().msaaColorAttachment->getImageView());
        }
        attachments.push_back(*(this->depthBuffer->getImageView()));
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *(renderPass->getRenderPass());
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;
        if(vkCreateFramebuffer(this->logicalDevice->getLogicalDevice(),&framebufferInfo,nullptr,&this->framebuffer) != VK_SUCCESS){
            ANTH_LOGE("Failed to create framebuffer");
            return false;
        }
        this->framebufferExtent.height = framebufferInfo.height;
        this->framebufferExtent.width = framebufferInfo.width;

        ANTH_LOGI("Framebuffer created, ", "Width=",framebufferInfo.width," Height=",framebufferInfo.height);
        return true;
    }
    bool AnthemFramebuffer::recreateFramebuffer(){
        return this->createFromColorAttachment(&(this->colorAttachment),this->ctRenderPass);
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
    VkFramebuffer* AnthemFramebuffer::getFramebuffer() const{
        return (VkFramebuffer*) & (this->framebuffer);
    }
    VkExtent2D AnthemFramebuffer::getExtent() const {
        return this->framebufferExtent;
    }
}