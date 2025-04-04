#include "../../../include/core/drawing/AnthemDrawingCommandHelper.h"

namespace Anthem::Core{
    bool AnthemDrawingCommandHelper::initializeHelper(){
        for(auto i=0;i<config->vkcfgMaxImagesInFlight;i++){
            uint32_t frameCmdBufIdx;
            if(!cmdBufs->createCommandBuffer(&frameCmdBufIdx)){
                ANTH_LOGE("Failed to request command buffer");
                return false;
            }
            this->commandBufferIdx.push_back(frameCmdBufIdx);
        }
        ANTH_LOGI("Drawing command helper initialized");
        return true;
    }
    bool AnthemDrawingCommandHelper::startRenderPass(AnthemCommandManagerRenderPassStartInfo* startInfo,uint32_t frameIdx){
        //this->cmdBufs->startCommandRecording(commandBufferIdx[frameIdx]);
        ANTH_LOGI("Cmdbuf binds");
        ANTH_ASSERT(swapChain != nullptr,"Swap chain not specified");
        
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = *(startInfo->renderPass->getRenderPass());
        renderPassBeginInfo.framebuffer = *(startInfo->framebufferList->getFramebuffer());
        renderPassBeginInfo.renderArea.offset = {0,0};
        renderPassBeginInfo.renderArea.extent = *(swapChain->getSwapChainExtent());
        
        auto renderPassClearValue = startInfo->renderPass->getDefaultClearValue();
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(renderPassClearValue->size());
        renderPassBeginInfo.pClearValues = renderPassClearValue->data();

        ANTH_LOGV("Starting render pass");
        auto cmdBuf = cmdBufs->getCommandBuffer(commandBufferIdx[frameIdx]);
        vkCmdBeginRenderPass(*cmdBuf,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
        ANTH_LOGV("Render pass started");
        return true;
    }

    bool AnthemDrawingCommandHelper::endRenderPass(uint32_t frameIdx){
        auto cmdBuf = cmdBufs->getCommandBuffer(commandBufferIdx[frameIdx]);
        vkCmdEndRenderPass(*cmdBuf);
        //this->cmdBufs->endCommandRecording(commandBufferIdx[frameIdx]);
        return true;
    }
    bool AnthemDrawingCommandHelper::demoDrawCommand(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport,AnthemVertexBuffer* vbuf,uint32_t frameIdx){
        auto cmdBuf = cmdBufs->getCommandBuffer(commandBufferIdx[frameIdx]);
        ANTH_ASSERT(pipeline != nullptr,"Pipeline not specified");
        ANTH_ASSERT(viewport != nullptr,"Viewport not specified");
        vkCmdSetViewport(*cmdBuf,0,1,viewport->getViewport());
        vkCmdSetScissor(*cmdBuf,0,1,viewport->getScissor());
        vkCmdBindPipeline(*cmdBuf,VK_PIPELINE_BIND_POINT_GRAPHICS,*(pipeline->getPipeline()));
        
        if(vbuf!=nullptr){
            VkBuffer vertexBuffers[] = {
                *(vbuf->getDestBufferObject())
            };
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(*cmdBuf,0,1,vertexBuffers,offsets);
            vkCmdDraw(*cmdBuf,3,1,0,0);
        }else{
            ANTH_LOGW("Not using vertex buffer");
            vkCmdDraw(*cmdBuf,3,1,0,0);
        }
        return true;
    }
    bool AnthemDrawingCommandHelper::demoDrawCommand2(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport,AnthemVertexBuffer* vbuf,AnthemIndexBuffer* ibuf,uint32_t frameIdx){
        auto cmdBuf = cmdBufs->getCommandBuffer(commandBufferIdx[frameIdx]);
        ANTH_ASSERT(pipeline != nullptr,"Pipeline not specified");
        ANTH_ASSERT(viewport != nullptr,"Viewport not specified");
        vkCmdSetViewport(*cmdBuf,0,1,viewport->getViewport());
        vkCmdSetScissor(*cmdBuf,0,1,viewport->getScissor());
        vkCmdBindPipeline(*cmdBuf,VK_PIPELINE_BIND_POINT_GRAPHICS,*(pipeline->getPipeline()));

        VkBuffer vertexBuffers[] = {*(vbuf->getDestBufferObject())};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(*cmdBuf,0,1,vertexBuffers,offsets);
        vkCmdBindIndexBuffer(*cmdBuf, *(ibuf->getDestBufferObject()), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(*cmdBuf, static_cast<uint32_t>(ibuf->getIndexCount()), 1, 0, 0, 0);
        return true;
    }

    bool AnthemDrawingCommandHelper::demoDrawCommand3(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport,AnthemVertexBuffer* vbuf,
    AnthemIndexBuffer* ibuf,AnthemUniformBuffer* ubuf, AnthemDescriptorPool* descPool, uint32_t frameIdx){
        auto cmdBuf = cmdBufs->getCommandBuffer(commandBufferIdx[frameIdx]);
        ANTH_ASSERT(pipeline != nullptr,"Pipeline not specified");
        ANTH_ASSERT(viewport != nullptr,"Viewport not specified");
        vkCmdSetViewport(*cmdBuf,0,1,viewport->getViewport());
        vkCmdSetScissor(*cmdBuf,0,1,viewport->getScissor());
        vkCmdBindPipeline(*cmdBuf,VK_PIPELINE_BIND_POINT_GRAPHICS,*(pipeline->getPipeline()));

        VkBuffer vertexBuffers[] = {*(vbuf->getDestBufferObject())};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(*cmdBuf,0,1,vertexBuffers,offsets);
        vkCmdBindIndexBuffer(*cmdBuf, *(ibuf->getDestBufferObject()), 0, VK_INDEX_TYPE_UINT32);
        //ANTH_LOGI("Binding pipeline layout:",(long long)(pipeline->getPipelineLayout()));
        std::vector<VkDescriptorSet> descSets = {};
        descPool->getAllDescriptorSets(frameIdx,&descSets);
        vkCmdBindDescriptorSets(*cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, *(pipeline->getPipelineLayout()), 0,
            static_cast<uint32_t>(descSets.size()),descSets.data() , 0, nullptr);
        vkCmdDrawIndexed(*cmdBuf, static_cast<uint32_t>(ibuf->getIndexCount()), 1, 0, 0, 0);
        return true;
    }
}
    