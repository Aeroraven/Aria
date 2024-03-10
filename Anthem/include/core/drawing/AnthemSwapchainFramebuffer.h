#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemSwapChain.h"
#include "../base/AnthemLogicalDevice.h"
#include "../pipeline/AnthemRenderPass.h"
#include "./AnthemFramebuffer.h"

namespace Anthem::Core{
    class AnthemSwapchainFramebuffer{
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
        std::vector<AnthemFramebuffer> framebuffers;
        AnthemDepthBuffer* depthBuffer = nullptr;
    public:
        bool virtual setDepthBuffer(AnthemDepthBuffer* depthBuffer);
        bool virtual specifyLogicalDevice(const AnthemLogicalDevice* device);
        bool virtual createFramebuffersFromSwapChain(const AnthemSwapChain* swapChain,const AnthemRenderPass* renderPass);
        bool virtual destroyFramebuffers();
        const AnthemFramebuffer* getFramebufferObject(uint32_t index) const;
        AnthemFramebuffer* getFramebufferObjectUnsafe(uint32_t index);
    };
}