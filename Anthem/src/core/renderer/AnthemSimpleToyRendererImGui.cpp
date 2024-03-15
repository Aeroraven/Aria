#include "../../../include/core/renderer/AnthemSimpleToyRenderer.h"

namespace Anthem::Core {
    bool AnthemSimpleToyRenderer::exGetImGuiCommandBufferIndex(uint32_t frame, uint32_t* result) {
        *result = this->g_CommandBufferIdx[frame];
        return true;
    }
    bool AnthemSimpleToyRenderer::exGetImGuiDrawProgressSemaphore(uint32_t frame, AnthemSemaphore** result) {
        *result = this->g_RenderComplete[frame];
        return true;
    }
    bool AnthemSimpleToyRenderer::exDestroyImgui() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyRenderPass(this->logicalDevice->getLogicalDevice(), g_RenderPass, nullptr);
        vkDestroyDescriptorPool(this->logicalDevice->getLogicalDevice(), g_DescriptorPool, nullptr);
        return true;
    }
	bool AnthemSimpleToyRenderer::exInitImGui() {
        {
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1;
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            auto err = vkCreateDescriptorPool(this->logicalDevice->getLogicalDevice(), &pool_info, nullptr, &g_DescriptorPool);
            exImGuiCheckStat(err);
        }
        {
            VkAttachmentDescription attachment = {};
            attachment.format = *this->swapChain->getFormat();
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            VkAttachmentReference color_attachment = {};
            color_attachment.attachment = 0;
            color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription depthAttachment = {};
            VkAttachmentReference depthAttachmentRef{};
            depthAttachment.format = VK_FORMAT_D32_SFLOAT;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_attachment;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            VkSubpassDependency dependency = {};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;


            std::vector<VkAttachmentDescription> desc = { attachment,depthAttachment };
            VkRenderPassCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            info.attachmentCount = 2;
            info.pAttachments = desc.data();
            info.subpassCount = 1;
            info.pSubpasses = &subpass;
            info.dependencyCount = 1;
            info.pDependencies = &dependency;
            auto err = vkCreateRenderPass(this->logicalDevice->getLogicalDevice(), &info, nullptr, &this->g_RenderPass);
            exImGuiCheckStat(err);

        }
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)this->instance->getWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = *this->instance->getInstance();
        init_info.PhysicalDevice = this->phyDevice->getPhysicalDevice();
        init_info.Device = this->logicalDevice->getLogicalDevice();
        init_info.QueueFamily = this->phyDevice->getPhyQueueGraphicsFamilyIndice().value();
        init_info.Queue = this->logicalDevice->getComputeQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = g_DescriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = exImGuiCheckStat;
        ImGui_ImplVulkan_Init(&init_info, g_RenderPass);

        // Sync & Rd
        this->g_CommandBufferIdx = new uint32_t[this->config->VKCFG_MAX_IMAGES_IN_FLIGHT];
        this->g_RenderComplete = new AnthemSemaphore * [this->config->VKCFG_MAX_IMAGES_IN_FLIGHT];
        for (int i = 0; i < this->config->VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
            this->drAllocateCommandBuffer(&this->g_CommandBufferIdx[i]);
            this->createSemaphore(&this->g_RenderComplete[i]);
        }
        return true;
	}

    bool AnthemSimpleToyRenderer::exRenderImGui(uint32_t frame, AnthemSwapchainFramebuffer* fb, std::array<float, 4> clearColor, ImDrawData* drawData) {
        VkClearValue clearValue[2];
        clearValue[0].color = { {clearColor[0], clearColor[1], clearColor[2], clearColor[3]} };
        clearValue[1].depthStencil = { 1.0f, 0 };

        this->commandBuffers->resetCommandBuffer(this->g_CommandBufferIdx[frame]);
        this->drStartCommandRecording(this->g_CommandBufferIdx[frame]);
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = this->g_RenderPass;
            info.framebuffer = *fb->getFramebufferObject(frame)->getFramebuffer();
            info.renderArea.extent.width = this->swapChain->getSwapChainExtentWidth();
            info.renderArea.extent.height = this->swapChain->getSwapChainExtentHeight();
            info.clearValueCount = 2;
            info.pClearValues = clearValue;
            vkCmdBeginRenderPass(*this->commandBuffers->getCommandBuffer(this->g_CommandBufferIdx[frame]), &info, VK_SUBPASS_CONTENTS_INLINE);
        }
        ImGui_ImplVulkan_RenderDrawData(drawData, *this->commandBuffers->getCommandBuffer(this->g_CommandBufferIdx[frame]));
        this->drEndRenderPass(this->g_CommandBufferIdx[frame]);

        this->drEndCommandRecording(this->g_CommandBufferIdx[frame]);
        return true;
    }

}