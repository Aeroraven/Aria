#include "../../../include/core/pipeline/AnthemViewport.h"

namespace Anthem::Core{
    bool AnthemViewport::specifyLogicalDevice(const AnthemLogicalDevice* device){
        this->logicalDevice = device;
        return true;
    }
    bool AnthemViewport::specifySwapChain(const AnthemSwapChain* swapChain){
        this->swapChain = swapChain;
        return true;
    }
    bool AnthemViewport::prepareViewportState(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->swapChain != nullptr,"Swap chain not specified");
        this->viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)(this->swapChain->getSwapChainExtentWidth()),
            .height = (float)(this->swapChain->getSwapChainExtentHeight()),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        this->scissor = {
            .offset = {0,0},
            .extent = *(this->swapChain->getSwapChainExtent()),
        };
        this->viewportPrepared = true;
        return true;
    }
    bool AnthemViewport::getViewportStateCreateInfo(VkPipelineViewportStateCreateInfo* outCreateInfo) const{
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->swapChain != nullptr,"Swap chain not specified");
        ANTH_ASSERT(this->viewportPrepared,"Viewport not prepared");

        outCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        outCreateInfo->pNext = nullptr;
        outCreateInfo->flags = 0;
        outCreateInfo->viewportCount = 1;
        outCreateInfo->pViewports = &(this->viewport);
        outCreateInfo->scissorCount = 1;
        outCreateInfo->pScissors = &(this->scissor);
        return true;
    }
    const VkViewport* AnthemViewport::getViewport() const{
        ANTH_ASSERT(this->viewportPrepared,"Viewport not prepared");
        return &(this->viewport);
    }
    const VkRect2D* AnthemViewport::getScissor() const{
        ANTH_ASSERT(this->viewportPrepared,"Viewport not prepared");
        return &(this->scissor);
    }
}