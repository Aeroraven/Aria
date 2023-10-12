#include "../../../include/core/drawing/AnthemCommandManager.h"

namespace Anthem::Core{
    bool AnthemCommandManager::createCommandPool(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->phyDevice != nullptr,"Physical device not specified");

        auto graphicsFamilyIdx = this->phyDevice->getPhyQueueGraphicsFamilyIndice();
        ANTH_ASSERT(graphicsFamilyIdx.has_value(),"Graphics queue family index not specified");

        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = graphicsFamilyIdx.value();
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if(vkCreateCommandPool(this->logicalDevice->getLogicalDevice(),&poolCreateInfo,nullptr,&commandPool)!=VK_SUCCESS){
            ANTH_LOGE("Failed to create command pool");
            return false;
        }
        ANTH_LOGI("Command pool created");
        this->commandPoolCreated = true;
        return true;
    }
    bool AnthemCommandManager::destroyCommandPool(){
        ANTH_LOGI("Destroying command pool");
        vkDestroyCommandPool(this->logicalDevice->getLogicalDevice(),commandPool,nullptr);
        return true;
    }
    bool AnthemCommandManager::createCommandBuffer(){
        ANTH_ASSERT(this->commandPoolCreated,"Command pool not created");

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = this->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if(vkAllocateCommandBuffers(this->logicalDevice->getLogicalDevice(),&allocInfo,&commandBuffer)!=VK_SUCCESS){
            ANTH_LOGE("Failed to allocate command buffer");
            return false;
        }
        ANTH_LOGI("Command buffer allocated");
        return true;
    }
    bool AnthemCommandManager::destroyCommandBuffer(){
        ANTH_LOGI("Destroying command buffer");
        vkFreeCommandBuffers(this->logicalDevice->getLogicalDevice(),this->commandPool,1,&commandBuffer);
        return true;
    }
    bool AnthemCommandManager::resetCommandBuffer(){
        ANTH_ASSERT(this->commandPoolCreated,"Command pool not created");
        if(vkResetCommandBuffer(commandBuffer,0)!=VK_SUCCESS){
            ANTH_LOGE("Failed to reset command buffer");
            return false;
        }
        ANTH_LOGI("Command buffer reset");
        return true;
    }
    bool AnthemCommandManager::startCommandRecording(){
        ANTH_LOGI("Starting command recording");
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if(vkBeginCommandBuffer(commandBuffer,&beginInfo)!=VK_SUCCESS){
            ANTH_LOGE("Failed to begin command buffer recording");
            return false;
        }
        ANTH_LOGI("Command recording started");
        this->commandBufferStarted = true;
        return true;
    }
    bool AnthemCommandManager::endCommandRecording(){
        ANTH_LOGI("Ending command recording");
        if(vkEndCommandBuffer(commandBuffer)!=VK_SUCCESS){
            ANTH_LOGE("Failed to end command buffer recording");
            return false;
        }
        ANTH_LOGI("Command recording ended");
        this->commandBufferStarted = false;
        return true;
    }
    bool AnthemCommandManager::startRenderPass(AnthemCommandManagerRenderPassStartInfo* startInfo){
        ANTH_ASSERT(this->commandBufferStarted,"Command buffer not started");
        ANTH_ASSERT(swapChain != nullptr,"Swap chain not specified");

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = *(startInfo->renderPass->getRenderPass());
        renderPassBeginInfo.framebuffer = *(startInfo->framebufferList->getFramebuffer(startInfo->framebufferIdx));
        renderPassBeginInfo.renderArea.offset = {0,0};
        renderPassBeginInfo.renderArea.extent = *(swapChain->getSwapChainExtent());
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &(startInfo->clearValue);

        ANTH_LOGI("Starting render pass");
        vkCmdBeginRenderPass(commandBuffer,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
        ANTH_LOGI("Render pass started");
        return true;
    }

    bool AnthemCommandManager::endRenderPass(){
        ANTH_ASSERT(this->commandBufferStarted,"Command buffer not started");
        vkCmdEndRenderPass(commandBuffer);
        return true;
    }
    bool AnthemCommandManager::demoDrawCommand(AnthemGraphicsPipeline* pipeline,AnthemViewport* viewport){
        ANTH_ASSERT(this->commandBufferStarted,"Command buffer not started");
        ANTH_ASSERT(pipeline != nullptr,"Pipeline not specified");
        ANTH_ASSERT(viewport != nullptr,"Viewport not specified");

        vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,*(pipeline->getPipeline()));
        vkCmdSetViewport(commandBuffer,0,1,viewport->getViewport());
        vkCmdSetScissor(commandBuffer,0,1,viewport->getScissor());
        vkCmdDraw(commandBuffer,3,1,0,0);
        return true;
    }
    const VkCommandBuffer* AnthemCommandManager::getCommandBuffer() const{
        return &(this->commandBuffer);
    }
    
}