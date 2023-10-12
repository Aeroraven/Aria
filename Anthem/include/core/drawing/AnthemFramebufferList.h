#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemSwapChain.h"
#include "../base/AnthemLogicalDevice.h"
#include "../pipeline/AnthemRenderPass.h"

namespace Anthem::Core{
    class AnthemFramebufferList{
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
        std::vector<VkFramebuffer> framebuffers;

    public:
        bool virtual specifyLogicalDevice(const AnthemLogicalDevice* device);
        bool virtual createFramebuffersFromSwapChain(const AnthemSwapChain* swapChain,const AnthemRenderPass* renderPass);
        bool virtual destroyFramebuffers();
    };
}