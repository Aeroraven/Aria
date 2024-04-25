#include "../../../include/core/pipeline/AnthemRenderPass.h"

namespace Anthem::Core{
    const std::vector<VkClearValue>* AnthemRenderPass::getDefaultClearValue() const{
        return &this->defaultClearValue;
    }
    const uint32_t AnthemRenderPass::getFilteredAttachmentCnt(AnthemRenderPassCreatedAttachmentType tp) const{
        uint32_t ret = 0;
        for(auto& p:this->createdAttachmentType){
            if(p==tp){
                ret+=1;
            }
        }
        return ret;
    }
    const uint32_t AnthemRenderPass::getTotalAttachmentCnt() const{
        return static_cast<uint32_t>(this->createdAttachmentType.size());
    }
    const AnthemRenderPassCreatedAttachmentType AnthemRenderPass::getAttachmentType(uint32_t idx) const{
        return this->createdAttachmentType[idx];
    }
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
    const AnthemRenderPassSetupOption& AnthemRenderPass::getSetupOption() const{
        return this->setupOption;
    }
    bool AnthemRenderPass::createDemoRenderPass(bool retain){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->swapChain != nullptr,"Swap chain not specified"); 
        
        //Create Attachment Description
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = (this->swapChain->getSwapChainSurfaceFormat())->format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        if (retain) {
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        }
       
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (retain) {
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        this->renderPassAttachments.push_back(colorAttachment);

        //Create Depth Attachment Description
        VkAttachmentDescription depthAttachment = {};
        VkAttachmentReference depthAttachmentRef{};

        if(this->depthBuffer != nullptr){
            depthAttachment.format = this->depthBuffer->getDepthFormat();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            if (retain) { // TODO:
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if (retain) {
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            this->renderPassAttachments.push_back(depthAttachment);

            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else{
            ANTH_LOGW("Depth buffer not specified");
        }

        //Specify Tp (Compat)
        registerAttachmentType(AT_ARPCA_COLOR,this->setupOption.predefinedClearColor);
        registerAttachmentType(AT_ARPCA_DEPTH);


        //Create Attachment Reference
        VkAttachmentReference colorAttachmentReference = {};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        this->colorAttachmentReferences.push_back(colorAttachmentReference);

        //Create Subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(this->colorAttachmentReferences.size());
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
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(this->renderPassAttachments.size());
        renderPassCreateInfo.pAttachments = this->renderPassAttachments.data();
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

    bool AnthemRenderPass::createRenderPass(const AnthemRenderPassSetupOption& opt){
        //Assertions
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->swapChain != nullptr,"Swap chain not specified"); 
        ANTH_ASSERT(opt.renderPassUsage != AT_ARPAA_UNDEFINED,"renderPassUsage flag should not be undefined");
        ANTH_ASSERT(opt.msaaType != AT_ARPMT_UNDEFINED,"msaaType flag should not be undefined");
        if(opt.msaaType == AT_ARPMT_MSAA){
            ANTH_ASSERT(opt.colorAttachmentFormats.size() == 1,"todo: support multiple color attachments");
        }
        if(opt.renderPassUsage == AT_ARPAA_DEPTH_STENCIL_ONLY_PASS){
            ANTH_ASSERT(opt.colorAttachmentFormats.size() == 0, "colorAttachmentFormats should not contain element");
        }else{
            ANTH_ASSERT(opt.colorAttachmentFormats.size() >= 1, "colorAttachmentFormats should have at least one element");
        }

        this->setupOption = opt;

        //Create Attachment Description
        ANTH_LOGI("Assertion done");
        std::vector<VkAttachmentDescription> colorAttachmentList = {};
        for(int i=0;i<opt.colorAttachmentFormats.size();i++){
            VkAttachmentDescription colorAttachment = {};
            if(opt.colorAttachmentFormats[i].has_value()){
                const auto dfm = opt.colorAttachmentFormats[i].value();
                VkFormat tgtFm;
                if(dfm==AT_IF_SIGNED_FLOAT32){
                    tgtFm = VK_FORMAT_R32G32B32A32_SFLOAT;
                }
                else if(dfm==AT_IF_SRGB_UINT8){
                    tgtFm = VK_FORMAT_R8G8B8A8_SRGB;
                }
                else if(dfm==AT_IF_SBGR_UINT8){
                    tgtFm = VK_FORMAT_B8G8R8A8_SRGB;
                }
                else if (dfm == AT_IF_SIGNED_FLOAT32_MONO) {
                    tgtFm = VK_FORMAT_R32_SFLOAT;
                }
                else if (dfm == AT_IF_UNORM_UINT8) {
                    tgtFm = VK_FORMAT_R8G8B8A8_UNORM;
                }
                colorAttachment.format = tgtFm;
            }else{
                colorAttachment.format = (this->swapChain->getSwapChainSurfaceFormat())->format;
            }
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            if (opt.clearColorAttachmentOnLoad[i] == false) {
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                if (opt.renderPassUsage != AT_ARPAA_FINAL_PASS) {
                    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }
                else {
                    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                }
            }
            else {
                colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            }
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            
            if(opt.msaaType == AT_ARPMT_MSAA){
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            }
            if(opt.renderPassUsage == AT_ARPAA_FINAL_PASS){
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }else{
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            colorAttachmentList.push_back(colorAttachment);
        }
        //Create Msaa Attachment
        int colorAttachmentMsaaIndex = -1;
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
        }

        for(int i=0;i<opt.colorAttachmentFormats.size();i++){
            this->renderPassAttachments.push_back(colorAttachmentList[i]);
            registerAttachmentType(AT_ARPCA_COLOR, this->setupOption.clearColors[i]);
        }
        if(opt.msaaType == AT_ARPMT_MSAA){
            this->renderPassAttachments.push_back(colorAttachmentMsaa);
            registerAttachmentType(AT_ARPCA_COLOR_MSAA, this->setupOption.predefinedClearColor);
            colorAttachmentMsaaIndex = static_cast<uint32_t>(this->renderPassAttachments.size())-1;
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
            if (opt.clearDepthAttachmentOnLoad) {
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
            else {
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            if (opt.storeDepthValues) {
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            }
            else {
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }


            if (depthBuffer->isStencilEnabled()) {
                if (opt.clearStencilAttachmentOnLoad) {
                    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                }
                else {
                    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                }
            }
            else {
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }

            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            if(opt.renderPassUsage == AT_ARPAA_DEPTH_STENCIL_ONLY_PASS){
                if(!opt.preserveWritableDepth)depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            }
            this->renderPassAttachments.push_back(depthAttachment);
            registerAttachmentType(AT_ARPCA_DEPTH);

            depthAttachmentRef.attachment = static_cast<uint32_t>(this->renderPassAttachments.size())-1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else{
            ANTH_LOGW("Depth buffer not specified");
        }

        //Create Attachment Reference
        std::vector<VkAttachmentReference> colorAttachmentReferenceList = {};
        for(int i=0;i<opt.colorAttachmentFormats.size();i++){
            VkAttachmentReference colorAttachmentReference = {};
            colorAttachmentReference.attachment = i;
            colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentReferenceList.push_back(colorAttachmentReference);
        }
        
        VkAttachmentReference colorAttachmentMsaaReference = {};
        colorAttachmentMsaaReference.attachment = colorAttachmentMsaaIndex;
        colorAttachmentMsaaReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        //Create Subpass
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        if(opt.msaaType == AT_ARPMT_NO_MSAA){
            subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferenceList.size());
            subpass.pColorAttachments = colorAttachmentReferenceList.data();
        }else{
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentMsaaReference;
            subpass.pResolveAttachments = &colorAttachmentReferenceList[0];
        }

        subpass.pDepthStencilAttachment = (depthBuffer == nullptr) ? nullptr : &depthAttachmentRef;

        //Create Subpass Dependencies
        std::vector<VkSubpassDependency> subpassDependency = {};

        if(opt.renderPassUsage == AT_ARPAA_FINAL_PASS){
            subpassDependency.resize(1);
            subpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency[0].dstSubpass = 0;
            subpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            subpassDependency[0].srcAccessMask = 0;
            subpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            subpassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }else if(opt.renderPassUsage == AT_ARPAA_INTERMEDIATE_PASS){
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
            subpassDependency[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            subpassDependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }else if(opt.renderPassUsage == AT_ARPAA_DEPTH_STENCIL_ONLY_PASS){
            subpassDependency.resize(2);
            subpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency[0].dstSubpass = 0;
            subpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            subpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            subpassDependency[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            subpassDependency[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            subpassDependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            subpassDependency[1].srcSubpass = 0;
            subpassDependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            subpassDependency[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            subpassDependency[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            subpassDependency[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            subpassDependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }


        //Create Render Pass
        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(this->renderPassAttachments.size());
        renderPassCreateInfo.pAttachments = this->renderPassAttachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependency.size());
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