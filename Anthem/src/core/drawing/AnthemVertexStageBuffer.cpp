#include "../../../include/core/drawing/AnthemVertexStageBuffer.h"

namespace Anthem::Core{
    bool AnthemVertexStageBuffer::bindBufferInternal(AnthemVertexStageBufferProp* bufProp){
        //Bind Memory
        auto result = vkBindBufferMemory(this->logicalDevice->getLogicalDevice(),bufProp->buffer,bufProp->bufferMem,0);
        if(result!=VK_SUCCESS){
            ANTH_LOGE("Failed to bind buffer memory");
            return false;
        }
        ANTH_LOGI("Buffer memory binded");
        return true;
    }
    bool AnthemVertexStageBuffer::copyStagingToVertexBuffer(){
        //Copy data from CPUMEM to Staging Buffer
        ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
        
        void* rawBufferData;
        this->getRawBufferData(&rawBufferData);
        ANTH_ASSERT(rawBufferData,"Raw buffer data is nullptr!");

        void* data;
        vkMapMemory(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.bufferMem,0,this->calculateBufferSize(),0,&data);
        memcpy(data,rawBufferData,this->calculateBufferSize());
        vkUnmapMemory(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.bufferMem);
        ANTH_LOGI("Data copied to staging buffer");

        //Allocate command buffer
        uint32_t cmdBufIdx;
        this->cmdBufs->createCommandBuffer(&cmdBufIdx);
        ANTH_LOGI("Command buffer created");
        this->cmdBufs->startCommandRecording(cmdBufIdx);
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = this->calculateBufferSize();
        auto cmdBuf = this->cmdBufs->getCommandBuffer(cmdBufIdx);
        ANTH_LOGI("Command buffer recording started");
        vkCmdCopyBuffer(*cmdBuf,this->stagingBuffer.buffer,this->dstBuffer.buffer,1,&copyRegion);
        this->cmdBufs->endCommandRecording(cmdBufIdx);

        ANTH_LOGI("Command buffer recording ended");
        //Submit Command
        this->cmdBufs->submitTaskToGraphicsQueue(cmdBufIdx,true);
        this->cmdBufs->freeCommandBuffer(cmdBufIdx);

        //Free staging buffer
        vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.buffer,nullptr);
        vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.bufferMem,nullptr);
        ANTH_LOGI("Staging buffer freed");
        return true;
    }
    
    bool AnthemVertexStageBuffer::createBufferInternal(AnthemVertexStageBufferProp* bufProp, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memProp){
        ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
        ANTH_LOGI("Creating buffer");
        bufProp->bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufProp->bufferCreateInfo.size = this->calculateBufferSize();
        bufProp->bufferCreateInfo.usage = usage;
        bufProp->bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufProp->bufferCreateInfo.pNext = nullptr;
        if(vkCreateBuffer(this->logicalDevice->getLogicalDevice(),&(bufProp->bufferCreateInfo),nullptr,&(bufProp->buffer))!=VK_SUCCESS){
            ANTH_LOGE("Failed to create buffer");
            return false;
        }
        ANTH_LOGI("Buffer created");
        VkMemoryRequirements memReq = {};
        vkGetBufferMemoryRequirements(this->logicalDevice->getLogicalDevice(),bufProp->buffer,&memReq);
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = this->phyDevice->findMemoryType(memReq.memoryTypeBits,memProp);
        if(vkAllocateMemory(this->logicalDevice->getLogicalDevice(),&allocInfo,nullptr,&(bufProp->bufferMem))!=VK_SUCCESS){
            ANTH_LOGE("Failed to allocate memory");
            return false;
        }
        ANTH_LOGI("Memory allocated");
        this->bindBufferInternal(bufProp);
        return true;
    }
    bool AnthemVertexStageBuffer::destroyBuffer(){
        ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
        vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->dstBuffer.bufferMem,nullptr);
        vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),this->dstBuffer.buffer,nullptr);
        return true;
    }
}
