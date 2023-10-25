#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemSwapChain.h"
#include "../base/AnthemLogicalDevice.h"
#include "../base/AnthemSwapChain.h"
#include "../drawing/image/AnthemDepthBuffer.h"
#include "../drawing/image/AnthemImage.h"

namespace Anthem::Core{
    enum AnthemRenderPassAttachmentAccess{
        AT_ARPAA_UNDEFINED,
        AT_ARPAA_FINAL_PASS,
        AT_ARPAA_INTERMEDIATE_PASS
    };
    enum AnthemRenderPassMultisampleType{
        AT_ARPMT_UNDEFINED,
        AT_ARPMT_NO_MSAA,
        AT_ARPMT_MSAA,
    };

    struct AnthenRenderPassSetupOption{
        //Infos
        AnthemRenderPassAttachmentAccess attachmentAccess;
        AnthemRenderPassMultisampleType msaaType;

        //Objects
        AnthemImage* msaaColorAttachment = nullptr;
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_16_BIT;
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
        AnthenRenderPassSetupOption setupOption;
    public:
        bool virtual specifyLogicalDevice(AnthemLogicalDevice* device);
        bool virtual specifySwapChain(AnthemSwapChain* swapChain);
        bool virtual createDemoRenderPass();
        bool virtual createRenderPass(const AnthenRenderPassSetupOption& opt);
        bool virtual destroyRenderPass();
        bool virtual setDepthBuffer(AnthemDepthBuffer* depthBuffer);
        const VkRenderPass* getRenderPass() const;
        const AnthenRenderPassSetupOption& getSetupOption() const;
    };
}