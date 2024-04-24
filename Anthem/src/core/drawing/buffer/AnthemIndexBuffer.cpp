#include "../../../../include/core/drawing/buffer/AnthemIndexBuffer.h"

namespace Anthem::Core{
    bool AnthemIndexBuffer::setIndices(std::vector<uint32_t> indices){
        this->indices = indices;
        return true;
    }
    bool AnthemIndexBuffer::createBuffer(){
        this->createBufferInternal(&(this->stagingBuffer),VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        this->createBufferInternal(&(this->dstBuffer), (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        this->copyStagingToVertexBuffer();
        return true;
    }
    uint32_t AnthemIndexBuffer::calculateBufferSize(){
        return static_cast<uint32_t>(this->indices.size())*sizeof(uint32_t);
    }
    void AnthemIndexBuffer::getRawBufferData(void** dataDst){
        *dataDst = (void*)this->indices.data();
    }
    uint32_t AnthemIndexBuffer::getIndexCount(){
        return static_cast<uint32_t>(this->indices.size());
    }
}