#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemSwapChain.h"
#include "../base/AnthemLogicalDevice.h"
#include "../pipeline/AnthemRenderPass.h"
#include "../drawing/image/AnthemImage.h"
#include "../drawing/image/IAnthemImageViewContainer.h"

namespace Anthem::Core{
    enum AnthemFramebufferUsage{
        AT_AFU_UNDEFINED,
        AT_AFU_OFFSCREEN,
        AT_AFU_SWAPCHAIN
    };

    class AnthemFramebuffer{
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
        VkFramebuffer framebuffer = nullptr;
        std::vector<const IAnthemImageViewContainer*> colorAttachment = {};
        AnthemDepthBuffer* depthBuffer = nullptr;
        const AnthemRenderPass* ctRenderPass = nullptr;
        AnthemFramebufferUsage framebufferUsage = AT_AFU_UNDEFINED;
        VkExtent2D framebufferExtent;
    public:
        bool virtual setDepthBuffer(AnthemDepthBuffer* depthBuffer);
        bool virtual specifyLogicalDevice(const AnthemLogicalDevice* device);

        bool virtual createFromSwapchainImageView(VkImageView* swapChainImageView, const AnthemRenderPass* renderPass, int height, int width);
        bool virtual createFromColorAttachment(const std::vector<const IAnthemImageViewContainer*>* colorImages, const AnthemRenderPass* renderPass);
        bool virtual destroyFramebuffers();
        bool virtual recreateFramebuffer();
        VkFramebuffer* getFramebuffer() const;
        VkExtent2D getExtent() const;
    };
}