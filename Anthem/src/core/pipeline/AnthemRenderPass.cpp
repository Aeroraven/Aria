#include "../../../include/core/pipeline/AnthemRenderPass.h"

namespace Anthem::Core{
    bool AnthemRenderPass::specifyLogicalDevice(AnthemLogicalDevice* device){
        this->logicalDevice = device;
        return true;
    }
    bool AnthemRenderPass::specifySwapChain(AnthemSwapChain* swapChain){
        this->swapChain = swapChain;
        return true;
    }
    bool AnthemRenderPass::destroyRenderPass(){
        vkDestroyRenderPass(this->logicalDevice->getLogicalDevice(),this->renderPass,nullptr);
        this->renderPassCreated = false;
        return true;
    }
    const VkRenderPass* AnthemRenderPass::getRenderPass() const{
        ANTH_ASSERT(this->renderPassCreated,"Render pass not created");
        return &(this->renderPass);
    }
    bool AnthemRenderPass::createDemoRenderPass(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->swapChain != nullptr,"Swap chain not specified"); 

        //Create Attachment Description
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = (this->swapChain->getSwapChainSurfaceFormat())->format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        this->colorAttachments.push_back(colorAttachment);

        //Create Attachment Reference
        VkAttachmentReference colorAttachmentReference = {};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        this->colorAttachmentReferences.push_back(colorAttachmentReference);

        //Create Subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = this->colorAttachmentReferences.size();
        subpass.pColorAttachments = this->colorAttachmentReferences.data();

        //Create Render Pass
        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = this->colorAttachments.size();
        renderPassCreateInfo.pAttachments = this->colorAttachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;

        auto result = vkCreateRenderPass(this->logicalDevice->getLogicalDevice(),&renderPassCreateInfo,nullptr,&(this->renderPass));
        if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to create render pass",result);
            return false;
        }
        this->renderPassCreated = true;
        ANTH_LOGI("Render pass created");
        return true;
    }
}