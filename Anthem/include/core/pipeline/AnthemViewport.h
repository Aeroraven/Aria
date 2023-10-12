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
        bool prepareViewportState();
        
        const VkViewport* getViewport();
        const VkRect2D* getScissor();
        bool getViewportStateCreateInfo(VkPipelineViewportStateCreateInfo* outCreateInfo) const;
    };

}