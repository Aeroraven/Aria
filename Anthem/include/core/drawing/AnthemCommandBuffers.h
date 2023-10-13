#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../utils/AnthemUtlSwapChainReqBase.h"
#include "../utils/AnthemUtlAppConfigReqBase.h"
#include "../pipeline/AnthemRenderPass.h"
#include "../drawing/AnthemFramebufferList.h"
#include "../pipeline/AnthemGraphicsPipeline.h"
#include "../pipeline/AnthemViewport.h"
#include "../drawing/AnthemVertexBuffer.h"

namespace Anthem::Core{

    struct AnthemCommandManagerRenderPassStartInfo{
        AnthemRenderPass* renderPass;
        AnthemFramebufferList* framebufferList;
        uint32_t framebufferIdx;
        VkClearValue clearValue;
    };

    class AnthemCommandBuffers: public Util::AnthemUtlLogicalDeviceReqBase, 
        public Util::AnthemUtlPhyDeviceReqBase, public Util::AnthemUtlSwapChainReqBase,
        public Util::AnthemUtlConfigReqBase{
    private:
        VkCommandPoolCreateInfo poolCreateInfo = {};
        VkCommandPool commandPool;

        std::vector<VkCommandBuffer> commandBuffer;

        bool commandPoolCreated = false;
        bool commandBufferStarted = false;
        bool renderPassStarted = false;
    public:
        bool virtual createCommandPool();    
        bool virtual destroyCommandPool();

        bool virtual createCommandBuffer();
        bool virtual destroyCommandBuffer();
        bool virtual resetCommandBuffer(uint32_t frameIdx);

        bool virtual startCommandRecording(uint32_t frameIdx);
        bool virtual endCommandRecording(uint32_t frameIdx);

        bool virtual startRenderPass(AnthemCommandManagerRenderPassStartInfo* startInfo,uint32_t frameIdx);
        bool virtual demoDrawCommand(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport,AnthemVertexBuffer* vbuf,uint32_t frameIdx);
        bool virtual endRenderPass(uint32_t frameIdx);

        const VkCommandBuffer* getCommandBuffer(uint32_t frameIdx) const;
    };
}