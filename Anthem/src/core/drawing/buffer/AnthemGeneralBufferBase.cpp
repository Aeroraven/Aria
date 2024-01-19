#include "../../../../include/core/drawing/buffer/AnthemGeneralBufferBase.h"
namespace Anthem::Core{
    bool AnthemGeneralBufferBase::createBufferInternal(AnthemGeneralBufferProp* bufProp, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memProp){
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
    bool AnthemGeneralBufferBase::bindBufferInternal(AnthemGeneralBufferProp* bufProp){
        //Bind Memory
        auto result = vkBindBufferMemory(this->logicalDevice->getLogicalDevice(),bufProp->buffer,bufProp->bufferMem,0);
        if(result!=VK_SUCCESS){
            ANTH_LOGE("Failed to bind buffer memory");
            return false;
        }
        ///ANTH_LOGI("Buffer memory binded");
        return true;
    }
    
}