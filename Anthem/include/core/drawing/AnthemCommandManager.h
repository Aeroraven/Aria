#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../utils/AnthemUtlSwapChainReqBase.h"
#include "../pipeline/AnthemRenderPass.h"
#include "../drawing/AnthemFramebufferList.h"
#include "../pipeline/AnthemGraphicsPipeline.h"
#include "../pipeline/AnthemViewport.h"

namespace Anthem::Core{

    struct AnthemCommandManagerRenderPassStartInfo{
        AnthemRenderPass* renderPass;
        AnthemFramebufferList* framebufferList;
        uint32_t framebufferIdx;
        VkClearValue clearValue;
    };

    class AnthemCommandManager: public Util::AnthemUtlLogicalDeviceReqBase, 
        public Util::AnthemUtlPhyDeviceReqBase, public Util::AnthemUtlSwapChainReqBase{
    private:
        VkCommandPoolCreateInfo poolCreateInfo = {};
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        bool commandPoolCreated = false;
        bool commandBufferStarted = false;
        bool renderPassStarted = false;
    public:
        bool virtual createCommandPool();    
        bool virtual destroyCommandPool();

        bool virtual createCommandBuffer();
        bool virtual destroyCommandBuffer();
        bool virtual resetCommandBuffer();

        bool virtual startCommandRecording();
        bool virtual endCommandRecording();

        bool virtual startRenderPass(AnthemCommandManagerRenderPassStartInfo* startInfo);
        bool virtual demoDrawCommand(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport);
        bool virtual endRenderPass();

        const VkCommandBuffer* getCommandBuffer() const;
    };
}