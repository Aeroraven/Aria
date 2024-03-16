#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemSwapChain.h"

namespace Anthem::Core{
    class AnthemViewport{
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
        const AnthemSwapChain* swapChain = nullptr;
        VkViewport viewport = {};
        VkRect2D scissor = {};
        bool viewportPrepared = false;
    public:
        bool specifyLogicalDevice(const AnthemLogicalDevice* device);
        bool specifySwapChain(const AnthemSwapChain* swapChain);
        bool prepareViewportStateFromSwapchain();
        bool prepareViewportStateCustom(float width,float height,float minDepth,float maxDepth);
        
        const VkViewport* getViewport() const;
        const VkRect2D* getScissor() const;
        bool getViewportStateCreateInfo(VkPipelineViewportStateCreateInfo* outCreateInfo) const;
    };

}