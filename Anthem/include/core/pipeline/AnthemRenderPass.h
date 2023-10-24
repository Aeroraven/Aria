#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemSwapChain.h"
#include "../base/AnthemLogicalDevice.h"
#include "../base/AnthemSwapChain.h"
#include "../drawing/image/AnthemDepthBuffer.h"

namespace Anthem::Core{
    enum AnthemRenderPassAttachmentAccess{
        AT_ARP_FINAL_PASS,
        AT_ARP_INTERMEDIATE_PASS
    };

    struct AnthenRenderPassSetupOption{
        AnthemRenderPassAttachmentAccess attachmentAccess;
    };

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
        bool virtual createRenderPass(const AnthenRenderPassSetupOption& opt);
        bool virtual destroyRenderPass();
        bool virtual setDepthBuffer(AnthemDepthBuffer* depthBuffer);
        const VkRenderPass* getRenderPass() const;
    };
}