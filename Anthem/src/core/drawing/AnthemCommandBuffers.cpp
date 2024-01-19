#include "../../../include/core/drawing/AnthemCommandBuffers.h"

namespace Anthem::Core{
    bool AnthemCommandBuffers::createCommandPool(){
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
    bool AnthemCommandBuffers::destroyCommandPool(){
        ANTH_LOGI("Destroying command pool");
        vkDestroyCommandPool(this->logicalDevice->getLogicalDevice(),commandPool,nullptr);
        return true;
    }
    bool AnthemCommandBuffers::createCommandBuffer(uint32_t* bufIdx){
        ANTH_ASSERT(this->commandPoolCreated,"Command pool not created");
        ANTH_ASSERT(this->config != nullptr,"Config not specified");
        this->commandBuffer.push_back(new VkCommandBuffer());

        //ANTH_LOGI("Creating command buffer");
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = this->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        if(vkAllocateCommandBuffers(this->logicalDevice->getLogicalDevice(),&allocInfo,  this->commandBuffer[this->commandBuffer.size()-1])!=VK_SUCCESS){
            ANTH_LOGE("Failed to allocate command buffer");
            return false;
        }
        *bufIdx = static_cast<uint32_t>(this->commandBuffer.size())-1;
        ANTH_LOGV("Command buffer allocated");
        return true;
    }
    bool AnthemCommandBuffers::resetCommandBuffer(uint32_t bufIdx){
        ANTH_ASSERT(this->commandPoolCreated,"Command pool not created");
        if(vkResetCommandBuffer(*(commandBuffer[bufIdx]),0)!=VK_SUCCESS){
            ANTH_LOGE("Failed to reset command buffer, Id=", bufIdx);
            return false;
        }
        ANTH_LOGV("Command buffer reset");
        return true;
    }
    bool AnthemCommandBuffers::freeCommandBuffer(uint32_t bufIdx){
        ANTH_ASSERT(this->commandPoolCreated,"Command pool not created");
        vkFreeCommandBuffers(this->logicalDevice->getLogicalDevice(),this->commandPool,1,commandBuffer[bufIdx]);
        ANTH_LOGV("Command buffer freed");
        return true;
    }
    bool AnthemCommandBuffers::startCommandRecording(uint32_t bufIdx){
        ANTH_LOGV("Starting command recording");
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if(vkBeginCommandBuffer(*(commandBuffer[bufIdx]),&beginInfo)!=VK_SUCCESS){
            ANTH_LOGE("Failed to begin command buffer recording");
            return false;
        }
        ANTH_LOGV("Command recording started");
        this->commandBufferStarted = true;
        return true;
    }
    bool AnthemCommandBuffers::endCommandRecording(uint32_t bufIdx){
        ANTH_LOGV("Ending command recording");
        if(vkEndCommandBuffer(*(commandBuffer[bufIdx]))!=VK_SUCCESS){
            ANTH_LOGE("Failed to end command buffer recording");
            return false;
        }
        ANTH_LOGV("Command recording ended");
        this->commandBufferStarted = false;
        return true;
    }
    
    const VkCommandBuffer* AnthemCommandBuffers::getCommandBuffer(uint32_t bufIdx) const{
        return (this->commandBuffer[bufIdx]);
    }
    bool AnthemCommandBuffers::submitTaskToGraphicsQueue(uint32_t bufIdx, bool waitingForIdle){
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        auto cmdBuf = this->getCommandBuffer(bufIdx);
        submitInfo.pCommandBuffers = cmdBuf;

        if(vkQueueSubmit(this->logicalDevice->getGraphicsQueue(),1,&submitInfo,VK_NULL_HANDLE)!=VK_SUCCESS){
            ANTH_LOGE("Failed to submit command buffer to graphics queue");
            return false;
        }
        if(waitingForIdle){
            vkQueueWaitIdle(this->logicalDevice->getGraphicsQueue());
        }
        return true;
    }
    
}