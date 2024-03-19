#include "../../../../include/core/drawing/buffer/AnthemGeneralBufferUtilBase.h"

namespace Anthem::Core{
    bool AnthemGeneralBufferUtilBase::createBufferInternalUt(const AnthemLogicalDevice* logicalDevice,const AnthemPhyDevice* phyDevice, 
        AnthemGeneralBufferProp* bufProp, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp,void* memAllocFlags, uint32_t size){
        bufProp->bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufProp->bufferCreateInfo.size = size ? size : this->calculateBufferSize();
        bufProp->bufferCreateInfo.usage = usage;
        bufProp->bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufProp->bufferCreateInfo.pNext = nullptr;

        if(vkCreateBuffer(logicalDevice->getLogicalDevice(),&(bufProp->bufferCreateInfo),nullptr,&(bufProp->buffer))!=VK_SUCCESS){
            ANTH_LOGE("Failed to create buffer");
            return false;
        }
        VkMemoryRequirements memReq = {};
        vkGetBufferMemoryRequirements(logicalDevice->getLogicalDevice(),bufProp->buffer,&memReq);
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = phyDevice->findMemoryType(memReq.memoryTypeBits,memProp);
        if (memAllocFlags != nullptr) {
            allocInfo.pNext = memAllocFlags;
        }
        if(vkAllocateMemory(logicalDevice->getLogicalDevice(),&allocInfo,nullptr,&(bufProp->bufferMem))!=VK_SUCCESS){
            ANTH_LOGE("Failed to allocate memory");
            return false;
        }
        ANTH_LOGI("Memory allocated");
        this->bindBufferInternalUt(logicalDevice,bufProp);
        return true;
    }
    bool AnthemGeneralBufferUtilBase::bindBufferInternalUt(const AnthemLogicalDevice* logicalDevice, AnthemGeneralBufferProp* bufProp){
        //Bind Memory
        auto result = vkBindBufferMemory(logicalDevice->getLogicalDevice(),bufProp->buffer,bufProp->bufferMem,0);
        if(result!=VK_SUCCESS){
            ANTH_LOGE("Failed to bind buffer memory");
            return false;
        }
        ANTH_LOGI("Buffer memory binded");
        return true;
    }
    bool AnthemGeneralBufferUtilBase::copyDataToBufferInternalUt(const AnthemLogicalDevice* logicalDevice, AnthemGeneralBufferProp* bufProp, void* data, uint32_t size, bool flush) const {
        void* dest;
        vkMapMemory(logicalDevice->getLogicalDevice(), bufProp->bufferMem, 0, VK_WHOLE_SIZE, 0, &dest);
        memcpy(dest, data, size);
        if (flush) {
            VkMappedMemoryRange range{};
            range.memory = bufProp->bufferMem;
            range.offset = 0;
            range.size = VK_WHOLE_SIZE;
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            auto result = vkFlushMappedMemoryRanges(logicalDevice->getLogicalDevice(), 1, &range);
            ANTH_ASSERT(result == VK_SUCCESS, "Failed to flush memory:", result);
        }
        vkUnmapMemory(logicalDevice->getLogicalDevice(), bufProp->bufferMem);
        return true;
    }
    VkDeviceAddress AnthemGeneralBufferUtilBase::getBufferDeviceAddressUt(const AnthemLogicalDevice* logicalDevice, const AnthemGeneralBufferProp* bufProp) const{
        VkBufferDeviceAddressInfoKHR bdaInfo = {};
        bdaInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
        bdaInfo.pNext = nullptr;
        bdaInfo.buffer = bufProp->buffer;
        return logicalDevice->vkCall_vkGetBufferDeviceAddressKHR(logicalDevice->getLogicalDevice(), &bdaInfo);
    }

}