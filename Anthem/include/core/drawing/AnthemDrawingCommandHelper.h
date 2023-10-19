#pragma once
#include "../base/AnthemBaseImports.h" 
#include "../utils/AnthemUtlAppConfigReqBase.h"
#include "../utils/AnthemUtlCommandBufferReqBase.h"
#include "../utils/AnthemUtlSwapChainReqBase.h"
#include "../drawing/AnthemFramebufferList.h"
#include "../pipeline/AnthemRenderPass.h"
#include "../pipeline/AnthemGraphicsPipeline.h"
#include "../drawing/AnthemIndexBuffer.h"
#include "../drawing/AnthemUniformBuffer.h"
#include "../drawing/AnthemDescriptorPool.h"

namespace Anthem::Core{
    struct AnthemCommandManagerRenderPassStartInfo{
        AnthemRenderPass* renderPass;
        AnthemFramebufferList* framebufferList;
        uint32_t framebufferIdx;
        VkClearValue clearValue;
        std::optional<VkClearValue> depthClearValue;
    };

    class AnthemDrawingCommandHelper: public Util::AnthemUtlConfigReqBase,
        public Util::AnthemUtlCommandBufferReqBase,public Util::AnthemUtlSwapChainReqBase{
    private:
        std::vector<uint32_t> commandBufferIdx;
        std::array<VkClearValue, 2> clearValuesTmpX{};
    public:
        bool virtual initializeHelper();
        bool virtual startRenderPass(AnthemCommandManagerRenderPassStartInfo* startInfo,uint32_t frameIdx);
        bool virtual demoDrawCommand(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport,AnthemVertexBuffer* vbuf,uint32_t frameIdx);
        bool virtual demoDrawCommand2(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport,AnthemVertexBuffer* vbuf,AnthemIndexBuffer* ibuf,uint32_t frameIdx);
        bool virtual demoDrawCommand3(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport,AnthemVertexBuffer* vbuf,AnthemIndexBuffer* ibuf,AnthemUniformBuffer* ubuf ,
            AnthemDescriptorPool* descPool,uint32_t frameIdx);
        bool virtual endRenderPass(uint32_t frameIdx);
    };
}