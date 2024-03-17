#include "../../../../include/core/drawing/buffer/AnthemGeneralBufferBase.h"
namespace Anthem::Core{
    bool AnthemGeneralBufferBase::createBufferInternal(AnthemGeneralBufferProp* bufProp, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp, void* memAllocFlags,uint32_t size){
        ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
        bufProp->bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufProp->bufferCreateInfo.size = size?size:this->calculateBufferSize();
        bufProp->bufferCreateInfo.usage = usage;
        bufProp->bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufProp->bufferCreateInfo.pNext = nullptr;
        if(vkCreateBuffer(this->logicalDevice->getLogicalDevice(),&(bufProp->bufferCreateInfo),nullptr,&(bufProp->buffer))!=VK_SUCCESS){
            ANTH_LOGE("Failed to create buffer");
            return false;
        }
        VkMemoryRequirements memReq = {};
        vkGetBufferMemoryRequirements(this->logicalDevice->getLogicalDevice(),bufProp->buffer,&memReq);
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = this->phyDevice->findMemoryType(memReq.memoryTypeBits,memProp);
        if (memAllocFlags != nullptr) {
            allocInfo.pNext = memAllocFlags;
        }
        if(vkAllocateMemory(this->logicalDevice->getLogicalDevice(),&allocInfo,nullptr,&(bufProp->bufferMem))!=VK_SUCCESS){
            ANTH_LOGE("Failed to allocate memory");
            return false;
        }
        ANTH_LOGI("Created buffer");
        this->bindBufferInternal(bufProp);
        return true;
    }
    bool AnthemGeneralBufferBase::bindBufferInternal(AnthemGeneralBufferProp* bufProp){
        auto result = vkBindBufferMemory(this->logicalDevice->getLogicalDevice(),bufProp->buffer,bufProp->bufferMem,0);
        if(result!=VK_SUCCESS){
            ANTH_LOGE("Failed to bind buffer memory");
            return false;
        }
        return true;
    }
    bool AnthemGeneralBufferBase::setupBufferBarrierInternal(VkCommandBuffer cmdBuf,AnthemGeneralBufferProp* bufProp, AnthemBufferBarrierProp* src, AnthemBufferBarrierProp* dst) {
        VkBufferMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = src->access;
        barrier.srcQueueFamilyIndex = src->queueFamily;
        barrier.dstAccessMask = dst->access;
        barrier.dstQueueFamilyIndex = dst->queueFamily;
        barrier.size = VK_WHOLE_SIZE;
        barrier.buffer = bufProp->buffer;

        vkCmdPipelineBarrier(cmdBuf, src->stage, dst->stage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        return true;
    }
    VkDeviceAddress AnthemGeneralBufferBase::getBufferDeviceAddress(AnthemGeneralBufferProp* bufProp) {
        VkBufferDeviceAddressInfoKHR bdaInfo = {};
        bdaInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
        bdaInfo.pNext = nullptr;
        bdaInfo.buffer = bufProp->buffer;
        return this->logicalDevice->vkCall_vkGetBufferDeviceAddressKHR(this->logicalDevice->getLogicalDevice(), &bdaInfo);
    }
    bool AnthemGeneralBufferBase::copyDataToBuffer(AnthemGeneralBufferProp* bufProp, void* data, uint32_t size, bool flush) {
        void* dest;
        vkMapMemory(this->logicalDevice->getLogicalDevice(), bufProp->bufferMem, 0, size, 0, &dest);
        memcpy(dest, data, size);
        if (flush) {
            VkMappedMemoryRange range{};
            range.memory = bufProp->bufferMem;
            range.offset = 0;
            range.size = size;
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            auto result = vkFlushMappedMemoryRanges(this->logicalDevice->getLogicalDevice(), 1, &range);
            ANTH_ASSERT(result == VK_SUCCESS, "Failed to flush memory:", result);
        }
        vkUnmapMemory(this->logicalDevice->getLogicalDevice(), bufProp->bufferMem);
        return true;
    }

}