#pragma once
#include "AnthemVertexStageBuffer.h"
#include "IAnthemVertexBufferAttrLayout.h"

namespace Anthem::Core{
    class AnthemVertexBuffer: public AnthemVertexStageBuffer, public virtual IAnthemVertexBufferAttrLayout {
    protected:
        uint32_t attributeNums = 0;
        uint32_t totalVertices = 0;
        char* rawBufferData;

        VkVertexInputBindingDescription vertexInputBindingDescription = {};
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescription = {};

    protected:
        void getRawBufferData(void** dataDst) override{
            *dataDst = this->rawBufferData;
        }
    protected:
        bool virtual getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc) = 0;
        bool virtual getInputAttrDescriptionInternal(std::vector<VkVertexInputAttributeDescription>* desc) = 0;
    public:
        uint32_t virtual getOffsets() = 0;
        const VkVertexInputBindingDescription* getVertexInputBindingDescription() const{
            return &(this->vertexInputBindingDescription);
        }
        const std::vector<VkVertexInputAttributeDescription>* getVertexInputAttributeDescription() const{
            return &(this->vertexInputAttributeDescription);
        }
        bool virtual prepareVertexInputInfo(VkPipelineVertexInputStateCreateInfo* info){
            if(!this->getInputBindingDescriptionInternal(&vertexInputBindingDescription)){
                ANTH_LOGE("Failed to get input binding description");
                return false;
            }
            if(!this->getInputAttrDescriptionInternal(&vertexInputAttributeDescription)){
                ANTH_LOGE("Failed to get input attribute description");
                return false;
            }
            info->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            info->vertexBindingDescriptionCount = 1;
            info->pVertexBindingDescriptions = &vertexInputBindingDescription;
            info->vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescription.size());
            info->pVertexAttributeDescriptions = vertexInputAttributeDescription.data();
            return true;
        }
        bool virtual createBuffer() override{
            this->createBufferInternal(&(this->stagingBuffer),VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            this->createBufferInternal(&(this->dstBuffer), (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            this->copyStagingToVertexBuffer();
            return true;
        }
        
    };

}