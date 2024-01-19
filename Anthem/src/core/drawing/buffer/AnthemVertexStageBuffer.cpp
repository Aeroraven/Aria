#include "../../../../include/core/drawing/buffer/AnthemVertexStageBuffer.h"

namespace Anthem::Core{
    
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
        //ANTH_LOGI("Data copied to staging buffer");

        //Allocate command buffer
        uint32_t cmdBufIdx;
        this->cmdBufs->createCommandBuffer(&cmdBufIdx);
        //ANTH_LOGI("Command buffer created");
        this->cmdBufs->startCommandRecording(cmdBufIdx);
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = this->calculateBufferSize();
        auto cmdBuf = this->cmdBufs->getCommandBuffer(cmdBufIdx);
        //ANTH_LOGI("Command buffer recording started");
        vkCmdCopyBuffer(*cmdBuf,this->stagingBuffer.buffer,this->dstBuffer.buffer,1,&copyRegion);
        this->cmdBufs->endCommandRecording(cmdBufIdx);

        //ANTH_LOGI("Command buffer recording ended");

        
        //Submit Command
        this->cmdBufs->submitTaskToGraphicsQueue(cmdBufIdx,true);
        this->cmdBufs->freeCommandBuffer(cmdBufIdx);

        //Free staging buffer
        vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.buffer,nullptr);
        vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.bufferMem,nullptr);
        //ANTH_LOGI("Staging buffer freed");
        return true;
    }
    
    bool AnthemVertexStageBuffer::destroyBuffer(){
        ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
        //ANTH_LOGI("Deleting Buffer");
        vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->dstBuffer.bufferMem,nullptr);
        vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),this->dstBuffer.buffer,nullptr);
        return true;
    }
}
