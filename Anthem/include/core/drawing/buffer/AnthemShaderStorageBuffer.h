#pragma once
#include "../../base/AnthemBaseImports.h"
#include "./AnthemBufferMemAligner.h"
#include "./AnthemGeneralBufferBase.h"
#include "./AnthemGeneralBufferProp.h"
#include "../../utils/AnthemUtlCommandBufferReqBase.h"

namespace Anthem::Core{

    struct AnthemShaderStorageBufferProp{
        std::vector<AnthemGeneralBufferProp> ssbo;
        AnthemGeneralBufferProp staging;
    };

    enum class AnthemSSBOUsage{
        AT_ASBU_UNDEFINED = 0,
        AT_ASBU_VERTEX = 1
    };
    
    class AnthemShaderStorageBuffer:public virtual AnthemGeneralBufferBase,public virtual Util::AnthemUtlCommandBufferReqBase{
    protected:
        AnthemShaderStorageBufferProp bufferProp;
        char* bufferData;
        uint32_t numCopies = 1;
        AnthemSSBOUsage usage = AnthemSSBOUsage::AT_ASBU_UNDEFINED;

    public:
        const std::vector<AnthemGeneralBufferProp>* getBuffers(){
            return &(this->bufferProp.ssbo);
        }
        uint32_t virtual getBufferSize() {
            return this->calculateBufferSize();
        }
        bool setupBarrier(VkCommandBuffer cmd,uint32_t copyId,AnthemBufferBarrierProp* src, AnthemBufferBarrierProp* dst) {
            return this->setupBufferBarrierInternal(cmd, &this->bufferProp.ssbo[copyId], src, dst);
        }
        bool specifyNumCopies(uint32_t numCopies){
            this->numCopies = numCopies;
            this->bufferProp.ssbo.resize(numCopies);
            return true;
        }
        bool specifyUsage(AnthemSSBOUsage bufferUsage){
            this->usage = bufferUsage;
            return true;
        }
        bool copyFromStagingToSSBO(uint32_t idx){
            uint32_t cmdIdx;
            this->cmdBufs->createCommandBuffer(&cmdIdx);
            this->cmdBufs->startCommandRecording(cmdIdx);
            VkBufferCopy cpInfo = {};
            cpInfo.size = this->getBufferSize();
            vkCmdCopyBuffer(*this->cmdBufs->getCommandBuffer(cmdIdx),this->bufferProp.staging.buffer,this->bufferProp.ssbo[idx].buffer,1,&cpInfo);
            this->cmdBufs->endCommandRecording(cmdIdx);
            this->cmdBufs->submitTaskToGraphicsQueue(cmdIdx,true);
            return true;
        }
        bool prepareStagingBuffer(){
            this->createBufferInternal(&this->bufferProp.staging,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            ANTH_LOGI("Creating Staging:", this->bufferProp.staging.buffer);
            vkMapMemory(this->logicalDevice->getLogicalDevice(), this->bufferProp.staging.bufferMem, 0, getBufferSize(), 0, &this->bufferProp.staging.mappedMem);
            memcpy(this->bufferProp.staging.mappedMem, this->bufferData, (size_t)getBufferSize());
            vkUnmapMemory(this->logicalDevice->getLogicalDevice(), this->bufferProp.staging.bufferMem);
            return true;
        }
        bool prepareSSBO(){
            ANTH_ASSERT(this->usage != AnthemSSBOUsage::AT_ASBU_UNDEFINED,"Undefined usage");
            auto usageBit = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            
            for(uint32_t i=0;i<this->numCopies;i++){
                if(this->usage == AnthemSSBOUsage::AT_ASBU_VERTEX){
                    this->createBufferInternal(&this->bufferProp.ssbo[i], 
                        (VkBufferUsageFlagBits)(usageBit), 
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                    this->copyFromStagingToSSBO(i);
                }
            }
            return true;
        }
        bool copyDataBack(int copyId,void* receivingRegion) {
            AnthemGeneralBufferProp backStaging;
            this->createBufferInternal(&backStaging, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            VkBufferCopy cpInfo = {};
            cpInfo.size = this->getBufferSize();
            vkMapMemory(logicalDevice->getLogicalDevice(), backStaging.bufferMem, 0, getBufferSize(), 0, &backStaging.mappedMem);
            uint32_t cmdIdx;
            this->cmdBufs->createCommandBuffer(&cmdIdx);
            this->cmdBufs->startCommandRecording(cmdIdx);
            vkCmdCopyBuffer(*this->cmdBufs->getCommandBuffer(cmdIdx), this->bufferProp.ssbo[copyId].buffer, backStaging.buffer, 1, &cpInfo);
            this->cmdBufs->endCommandRecording(cmdIdx);
            this->cmdBufs->submitTaskToGraphicsQueue(cmdIdx, true);
            memcpy(receivingRegion, backStaging.mappedMem, (size_t)getBufferSize());
            vkUnmapMemory(logicalDevice->getLogicalDevice(), backStaging.bufferMem);
            vkFreeMemory(logicalDevice->getLogicalDevice(), backStaging.bufferMem,nullptr);
            vkDestroyBuffer(logicalDevice->getLogicalDevice(), backStaging.buffer, nullptr);
            return true;
        }


        const VkBuffer* getDestBufferObject(uint32_t copyId) const {
            return &(this->bufferProp.ssbo.at(copyId).buffer);
        }
        bool createShaderStorageBuffer(){
            prepareStagingBuffer();
            prepareSSBO();
            return true;
        }
        bool destroyStagingBuffer(){
            vkFreeMemory(this->logicalDevice->getLogicalDevice(),bufferProp.staging.bufferMem,nullptr);
            vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),bufferProp.staging.buffer,nullptr);
            return true;
        }

        bool destroySSBO(){
            for(uint32_t i=0;i<this->numCopies;i++){
                vkFreeMemory(this->logicalDevice->getLogicalDevice(),bufferProp.ssbo.at(i).bufferMem,nullptr);
                vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),bufferProp.ssbo.at(i).buffer,nullptr);
            }
            return true;
        }
    };

    

}