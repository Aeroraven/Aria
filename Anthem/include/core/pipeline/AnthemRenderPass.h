#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemSwapChain.h"
#include "../base/AnthemLogicalDevice.h"
#include "../base/AnthemSwapChain.h"
#include "../drawing/image/AnthemDepthBuffer.h"
#include "../drawing/image/AnthemImage.h"

namespace Anthem::Core{
    enum AnthemRenderPassUsageType{
        AT_ARPAA_UNDEFINED,
        AT_ARPAA_FINAL_PASS,
        AT_ARPAA_INTERMEDIATE_PASS,
        AT_ARPAA_DEPTH_STENCIL_ONLY_PASS
    };
    enum AnthemRenderPassMultisampleType{
        AT_ARPMT_UNDEFINED,
        AT_ARPMT_NO_MSAA,
        AT_ARPMT_MSAA,
    };
    enum AnthemRenderPassCreatedAttachmentType{
        AT_ARPCA_COLOR,
        AT_ARPCA_DEPTH,
        AT_ARPCA_COLOR_MSAA
    };

    struct AnthemRenderPassSetupOption{
        //Infos
        AnthemRenderPassUsageType renderPassUsage = AT_ARPAA_FINAL_PASS;
        bool preserveWritableDepth = false;
        AnthemRenderPassMultisampleType msaaType = AT_ARPMT_NO_MSAA;
        std::vector<std::optional<AnthemImageFormat>> colorAttachmentFormats = {std::nullopt};

        //Objects
        AnthemImage* msaaColorAttachment = nullptr;
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_16_BIT;

        //Clear attachments
        std::vector<int> clearColorAttachmentOnLoad = { true };
        bool clearDepthAttachmentOnLoad = true;
        bool storeDepthValues = true;
        bool clearStencilAttachmentOnLoad = true;
        std::optional<std::array<float, 4>> predefinedClearColor = std::nullopt;
        std::vector<std::array<float, 4>> clearColors = { {0,0,0,0} };
    };

    class AnthemRenderPass{
    private:
        std::vector<VkAttachmentDescription> renderPassAttachments;
        std::vector<VkAttachmentReference> colorAttachmentReferences;
        std::vector<VkSubpassDescription> subpasses;
        VkRenderPass renderPass;
        bool renderPassCreated = false;

        AnthemLogicalDevice* logicalDevice = nullptr;
        AnthemSwapChain* swapChain = nullptr;
        AnthemDepthBuffer* depthBuffer = nullptr;
        AnthemRenderPassSetupOption setupOption;
        std::vector<AnthemRenderPassCreatedAttachmentType> createdAttachmentType = {};
        std::vector<VkClearValue> defaultClearValue = {};
    protected:
        bool registerAttachmentType(AnthemRenderPassCreatedAttachmentType tp,std::optional<std::array<float,4>> clearColor=std::nullopt){
            this->createdAttachmentType.push_back(tp);
            defaultClearValue.push_back(VkClearValue());
            if(tp == AT_ARPCA_COLOR || tp == AT_ARPCA_COLOR_MSAA){
                if (clearColor == std::nullopt) {
                    defaultClearValue.back().color = { {0.0f, 0.0f, 0.0f, 0.0f} };
                }
                else {
                    defaultClearValue.back().color = { {clearColor.value()[0],clearColor.value()[1],clearColor.value()[2],clearColor.value()[3]}};
                }

            }else{
                defaultClearValue.back().depthStencil = {1.0f, 0};
            }
            return true;
        }
    public:
        bool virtual specifyLogicalDevice(AnthemLogicalDevice* device);
        bool virtual specifySwapChain(AnthemSwapChain* swapChain);
        bool virtual createDemoRenderPass(bool retain=false);
        bool virtual createRenderPass(const AnthemRenderPassSetupOption& opt);
        bool virtual destroyRenderPass();
        bool virtual setDepthBuffer(AnthemDepthBuffer* depthBuffer);
        const VkRenderPass* getRenderPass() const;
        const AnthemRenderPassSetupOption& getSetupOption() const;
        const AnthemRenderPassCreatedAttachmentType getAttachmentType(uint32_t idx) const;
        const uint32_t getTotalAttachmentCnt() const;
        const uint32_t getFilteredAttachmentCnt(AnthemRenderPassCreatedAttachmentType tp) const;
        const std::vector<VkClearValue>* getDefaultClearValue() const;


    };
}