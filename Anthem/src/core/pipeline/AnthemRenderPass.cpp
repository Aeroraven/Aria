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
    const AnthenRenderPassSetupOption& AnthemRenderPass::getSetupOption() const{
        return this->setupOption;
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
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        this->colorAttachments.push_back(colorAttachment);

        //Create Depth Attachment Description
        VkAttachmentDescription depthAttachment = {};
        VkAttachmentReference depthAttachmentRef{};

        if(this->depthBuffer != nullptr){
            depthAttachment.format = this->depthBuffer->getDepthFormat();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            this->colorAttachments.push_back(depthAttachment);

            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else{
            ANTH_LOGW("Depth buffer not specified");
        }

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
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        //Create Subpass Dependencies
        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;

        //Create Render Pass
        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = this->colorAttachments.size();
        renderPassCreateInfo.pAttachments = this->colorAttachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        auto result = vkCreateRenderPass(this->logicalDevice->getLogicalDevice(),&renderPassCreateInfo,nullptr,&(this->renderPass));
        if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to create render pass",result);
            return false;
        }
        this->renderPassCreated = true;
        ANTH_LOGI("Render pass created");
        return true;
    }
    bool AnthemRenderPass::setDepthBuffer(AnthemDepthBuffer* depthBuffer){
        this->depthBuffer = depthBuffer;
        return true;
    }

    bool AnthemRenderPass::createRenderPass(const AnthenRenderPassSetupOption& opt){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->swapChain != nullptr,"Swap chain not specified"); 
        
        ANTH_ASSERT(opt.attachmentAccess != AT_ARPAA_UNDEFINED,"attachmentAccess flag should not be undefined");
        ANTH_ASSERT(opt.msaaType != AT_ARPMT_UNDEFINED,"msaaType flag should not be undefined");

        this->setupOption = opt;

        //Create Attachment Description
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = (this->swapChain->getSwapChainSurfaceFormat())->format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if(opt.attachmentAccess == AT_ARPAA_FINAL_PASS){
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }else{
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        
        //Create Msaa Attachment
        VkAttachmentDescription colorAttachmentMsaa = {}; 
        if(opt.msaaType == AT_ARPMT_MSAA){
            colorAttachmentMsaa.format = (this->swapChain->getSwapChainSurfaceFormat())->format;
            colorAttachmentMsaa.samples = opt.msaaSamples;
            colorAttachmentMsaa.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachmentMsaa.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachmentMsaa.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentMsaa.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachmentMsaa.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachmentMsaa.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        this->colorAttachments.push_back(colorAttachment);
        if(opt.msaaType == AT_ARPMT_MSAA){
            this->colorAttachments.push_back(colorAttachmentMsaa);
        }

        //Create Depth Attachment Description
        VkAttachmentDescription depthAttachment = {};
        VkAttachmentReference depthAttachmentRef{};

        if(this->depthBuffer != nullptr){
            depthAttachment.format = this->depthBuffer->getDepthFormat();
            if(opt.msaaType == AT_ARPMT_MSAA){
                depthAttachment.samples = opt.msaaSamples;
            }else{
                depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            }
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            this->colorAttachments.push_back(depthAttachment);

            depthAttachmentRef.attachment = this->colorAttachments.size()-1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else{
            ANTH_LOGW("Depth buffer not specified");
        }

        //Create Attachment Reference
        VkAttachmentReference colorAttachmentReference = {};
        VkAttachmentReference colorAttachmentMsaaReference = {};

        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorAttachmentMsaaReference.attachment = 1;
        colorAttachmentMsaaReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        //Create Subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        if(opt.msaaType == AT_ARPMT_NO_MSAA){
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentReference;
        }else{
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentMsaaReference;
            subpass.pResolveAttachments = &colorAttachmentReference;
        }

        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        //Create Subpass Dependencies
        std::vector<VkSubpassDependency> subpassDependency = {};

        if(opt.attachmentAccess == AT_ARPAA_FINAL_PASS){
            subpassDependency.resize(1);
            subpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency[0].dstSubpass = 0;
            subpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            subpassDependency[0].srcAccessMask = 0;
            subpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            subpassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }else if(opt.attachmentAccess == AT_ARPAA_INTERMEDIATE_PASS){
            subpassDependency.resize(2);
            subpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency[0].dstSubpass = 0;
            subpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            subpassDependency[0].srcAccessMask = 0;
            subpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            subpassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            subpassDependency[1].srcSubpass = 0;
            subpassDependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependency[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            subpassDependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            subpassDependency[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            subpassDependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }


        //Create Render Pass
        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = this->colorAttachments.size();
        renderPassCreateInfo.pAttachments = this->colorAttachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = subpassDependency.size();
        renderPassCreateInfo.pDependencies = subpassDependency.data();

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