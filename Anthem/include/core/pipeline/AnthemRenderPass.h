#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemSwapChain.h"
#include "../base/AnthemLogicalDevice.h"
#include "../base/AnthemSwapChain.h"
#include "../drawing/AnthemDepthBuffer.h"

namespace Anthem::Core{
    class AnthemRenderPass{
    private:
        std::vector<VkAttachmentDescription> colorAttachments;
        std::vector<VkAttachmentReference> colorAttachmentReferences;
        std::vector<VkSubpassDescription> subpasses;
        VkRenderPass renderPass;
        bool renderPassCreated = false;

        AnthemLogicalDevice* logicalDevice = nullptr;
        AnthemSwapChain* swapChain = nullptr;
        AnthemDepthBuffer* depthBuffer = nullptr;
    public:
        bool virtual specifyLogicalDevice(AnthemLogicalDevice* device);
        bool virtual specifySwapChain(AnthemSwapChain* swapChain);
        bool virtual createDemoRenderPass();
        bool virtual destroyRenderPass();
        bool virtual setDepthBuffer(AnthemDepthBuffer* depthBuffer);
        const VkRenderPass* getRenderPass() const;
    };
}