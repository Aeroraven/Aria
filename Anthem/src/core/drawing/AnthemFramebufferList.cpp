#include "../../../include/core/drawing/AnthemFramebufferList.h"

namespace Anthem::Core{
    bool AnthemFramebufferList::specifyLogicalDevice(const AnthemLogicalDevice* device){
        this->logicalDevice = device;
        return true;
    }
    bool AnthemFramebufferList::createFramebuffersFromSwapChain(const AnthemSwapChain* swapChain,const AnthemRenderPass* renderPass){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        this->framebuffers.resize(swapChain->getSwapChainImageViews()->size());
        for(size_t i = 0; i < swapChain->getSwapChainImageViews()->size(); i++){
            VkImageView attachments[] = {
                swapChain->getSwapChainImageViews()->at(i)
            };
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = *(renderPass->getRenderPass());
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChain->getSwapChainExtent()->width;
            framebufferInfo.height = swapChain->getSwapChainExtent()->height;
            framebufferInfo.layers = 1;
            if(vkCreateFramebuffer(this->logicalDevice->getLogicalDevice(),&framebufferInfo,nullptr,&this->framebuffers[i]) != VK_SUCCESS){
                ANTH_LOGE("Failed to create framebuffer");
                return false;
            }
            ANTH_LOGI("Framebuffer created, idx=",i, "Width=",framebufferInfo.width," Height=",framebufferInfo.height);
        }
        return true;
    }
    bool AnthemFramebufferList::destroyFramebuffers(){
        ANTH_LOGI("Destroying framebuffers");
        for(auto framebuffer : this->framebuffers){
            vkDestroyFramebuffer(this->logicalDevice->getLogicalDevice(),framebuffer,nullptr);
        }
        this->framebuffers.clear();
        return true;
    }
    const VkFramebuffer* AnthemFramebufferList::getFramebuffer(uint32_t index) const{
        ANTH_ASSERT(index < this->framebuffers.size(),"Invalid index");
        return &(this->framebuffers[index]);
    }
}