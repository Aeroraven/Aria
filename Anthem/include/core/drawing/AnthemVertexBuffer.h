#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../utils/AnthemUtlCommandBufferReqBase.h"

namespace Anthem::Core{
    struct AnthemVertexStageBufferProp{
        VkBuffer buffer;
        VkBufferCreateInfo bufferCreateInfo = {};
        VkDeviceMemory bufferMem;
    };
    
    class AnthemVertexBuffer: public Util::AnthemUtlLogicalDeviceReqBase,public Util::AnthemUtlPhyDeviceReqBase,public Util::AnthemUtlCommandBufferReqBase{
    protected:
        uint32_t attributeNums = 0;
        uint32_t totalVertices = 0;
        char* rawBufferData;

        AnthemVertexStageBufferProp vertBuffer;
        AnthemVertexStageBufferProp stagingBuffer;
        
        VkVertexInputBindingDescription vertexInputBindingDescription = {};
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescription = {};

    protected:
        uint32_t virtual calculateBufferSize() = 0;
    protected:
        bool virtual copyStagingToVertexBuffer(){
            //Copy data from CPUMEM to Staging Buffer
            ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
            ANTH_ASSERT(this->rawBufferData,"Raw buffer data is nullptr!");
            
            void* data;
            vkMapMemory(this->logicalDevice->getLogicalDevice(),this->stagingBuffer.bufferMem,0,this->calculateBufferSize(),0,&data);
            memcpy(data,this->rawBufferData,this->calculateBufferSize());
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
            vkCmdCopyBuffer(*cmdBuf,this->stagingBuffer.buffer,this->vertBuffer.buffer,1,&copyRegion);
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
        bool virtual bindBufferInternal(AnthemVertexStageBufferProp* bufProp){
            //Bind Memory
            auto result = vkBindBufferMemory(this->logicalDevice->getLogicalDevice(),bufProp->buffer,bufProp->bufferMem,0);
            if(result!=VK_SUCCESS){
                ANTH_LOGE("Failed to bind buffer memory");
                return false;
            }
            ANTH_LOGI("Buffer memory binded");
            return true;
        }
        bool virtual createBufferInternal(AnthemVertexStageBufferProp* bufProp, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memProp){
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
    public:
        bool virtual getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc) = 0;
        bool virtual getInputAttrDescriptionInternal(std::vector<VkVertexInputAttributeDescription>* desc) = 0;

        const VkBuffer* getVertexBufferObj() const{
            return &(this->vertBuffer.buffer);
        }
        const VkVertexInputBindingDescription* getVertexInputBindingDescription() const{
            return &(this->vertexInputBindingDescription);
        }
        const std::vector<VkVertexInputAttributeDescription>* getVertexInputAttributeDescription() const{
            return &(this->vertexInputAttributeDescription);
        }

        bool virtual freeMemory(){
            ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
            vkFreeMemory(this->logicalDevice->getLogicalDevice(),this->vertBuffer.bufferMem,nullptr);
            return true;
        }
        bool virtual copyBufferMemoryToDevice(){
            ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
            ANTH_ASSERT(this->rawBufferData,"Raw buffer data is nullptr!");
            this->bindBufferInternal(&(this->vertBuffer));
            void* data;
            vkMapMemory(this->logicalDevice->getLogicalDevice(),this->vertBuffer.bufferMem,0,this->calculateBufferSize(),0,&data);
            memcpy(data,this->rawBufferData,this->calculateBufferSize());
            vkUnmapMemory(this->logicalDevice->getLogicalDevice(),this->vertBuffer.bufferMem);
            ANTH_LOGI("Data copied to device");
            return true;
        }
        bool virtual allocateMemory(){
            return true;
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
            info->vertexAttributeDescriptionCount = vertexInputAttributeDescription.size();
            info->pVertexAttributeDescriptions = vertexInputAttributeDescription.data();
            return true;
        }
        bool virtual createBuffer(){
            this->createBufferInternal(&(this->stagingBuffer),VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            this->createBufferInternal(&(this->vertBuffer), (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            this->copyStagingToVertexBuffer();
            return true;
        }
        bool virtual destroyBuffer(){
            ANTH_ASSERT(this->logicalDevice,"Device is nullptr!");
            vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),this->vertBuffer.buffer,nullptr);
            return true;
        }
    };

    template<typename Tp,uint32_t Sz> struct AnthemVAOAttrDesc;

    template<typename... AVAOTpDesc> class AnthemVertexBufferImpl;
    template<typename... AttrTp,uint32_t... AttrSz>
    class AnthemVertexBufferImpl<AnthemVAOAttrDesc<AttrTp,AttrSz>...>:public AnthemVertexBuffer{
    static_assert(sizeof...(AttrSz)==sizeof...(AttrTp),"Invalid data size");
    private:
        uint32_t singleVertexSize = 0;
        std::array<uint32_t,sizeof...(AttrSz)> attrDims = {AttrSz...};
        std::array<uint32_t,sizeof...(AttrTp)> attrTpSize = {sizeof(AttrTp)...};

        std::array<bool,sizeof...(AttrTp)> isFloatType = {std::is_floating_point<AttrTp>::value...};
        std::array<bool,sizeof...(AttrTp)> isIntType = {std::is_integral<AttrTp>::value...};
        std::array<bool,sizeof...(AttrTp)> isUnsignedType = {std::is_unsigned<AttrTp>::value...};
  
        

    public:
        AnthemVertexBufferImpl(){
            this->singleVertexSize =0;

            for(auto i=0;i<sizeof...(AttrSz);i++){
                ANTH_LOGI("Param Size=",this->attrDims[i]*this->attrTpSize[i]);
                this->singleVertexSize+=this->attrDims[i]*this->attrTpSize[i];
            }
            ANTH_LOGI("Total Size=",this->singleVertexSize);
        }
        bool setTotalVertices(uint32_t vertexNum){
            ANTH_ASSERT(vertexNum>0,"Invalid vertex number");
            this->totalVertices = vertexNum;
            this->rawBufferData = new char[this->calculateBufferSize()];
            ANTH_LOGI("Allocated:",this->calculateBufferSize());
            return true;
        }
        bool insertData(int idx,std::array<AttrTp,AttrSz>... data){
            ANTH_ASSERT(idx<this->totalVertices,"Invalid index");

            //Convert Src Data to Void Pointers
            std::vector<void*> dataPtrs = {data.data()...};

            //Memcpy
            auto offset = idx*this->singleVertexSize;
            auto dataOffset = 0;
            for(auto i=0;i<sizeof...(AttrSz);i++){
                auto dataSz = this->attrTpSize.at(i)*this->attrDims.at(i);
                memcpy(this->rawBufferData+offset+dataOffset,dataPtrs[i],dataSz);
                dataOffset+=dataSz;
            }
            return true;
        }
        bool getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc) override{
            ANTH_ASSERT(desc,"Description is nullptr");
            desc->binding = 0;
            desc->stride = this->singleVertexSize;
            desc->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return true;
        }
        constexpr VkFormat getFormatFromTypeInfo(uint32_t attrIdx){
            if(this->isFloatType.at(attrIdx)){
                if(this->attrTpSize.at(attrIdx)==sizeof(float)){
                    if(this->attrDims.at(attrIdx)==1){
                        return VK_FORMAT_R32_SFLOAT;
                    }else if(this->attrDims.at(attrIdx)==2){
                        return VK_FORMAT_R32G32_SFLOAT;
                    }else if(this->attrDims.at(attrIdx)==3){
                        return VK_FORMAT_R32G32B32_SFLOAT;
                    }else if(this->attrDims.at(attrIdx)==4){
                        return VK_FORMAT_R32G32B32A32_SFLOAT;
                    }
                }else if(this->attrTpSize.at(attrIdx)==sizeof(double)){
                    if(this->attrDims.at(attrIdx)==1){
                        return VK_FORMAT_R64_SFLOAT;
                    }else if(this->attrDims.at(attrIdx)==2){
                        return VK_FORMAT_R64G64_SFLOAT;
                    }else if(this->attrDims.at(attrIdx)==3){
                        return VK_FORMAT_R64G64B64_SFLOAT;
                    }else if(this->attrDims.at(attrIdx)==4){
                        return VK_FORMAT_R64G64B64A64_SFLOAT;
                    }
                }
            }else if(this->isIntType.at(attrIdx)){
                if(this->isUnsignedType.at(attrIdx)){
                    if(this->attrTpSize.at(attrIdx)==sizeof(uint8_t)){
                        if(this->attrDims.at(attrIdx)==1){
                            return VK_FORMAT_R8_UINT;
                        }else if(this->attrDims.at(attrIdx)==2){
                            return VK_FORMAT_R8G8_UINT;
                        }else if(this->attrDims.at(attrIdx)==3){
                            return VK_FORMAT_R8G8B8_UINT;
                        }else if(this->attrDims.at(attrIdx)==4){
                            return VK_FORMAT_R8G8B8A8_UINT;
                        }
                    }else if(this->attrTpSize.at(attrIdx)==sizeof(uint16_t)){
                        if(this->attrDims.at(attrIdx)==1){
                            return VK_FORMAT_R16_UINT;
                        }else if(this->attrDims.at(attrIdx)==2){
                            return VK_FORMAT_R16G16_UINT;
                        }else if(this->attrDims.at(attrIdx)==3){
                            return VK_FORMAT_R16G16B16_UINT;
                        }else if(this->attrDims.at(attrIdx)==4){
                            return VK_FORMAT_R16G16B16A16_UINT;
                        }

                    }else if(this->attrTpSize.at(attrIdx)==sizeof(uint32_t)){
                        if(this->attrDims.at(attrIdx)==1){
                            return VK_FORMAT_R32_UINT;
                        }else if(this->attrDims.at(attrIdx)==2){
                            return VK_FORMAT_R32G32_UINT;
                        }else if(this->attrDims.at(attrIdx)==3){
                            return VK_FORMAT_R32G32B32_UINT;
                        }else if(this->attrDims.at(attrIdx)==4){
                            return VK_FORMAT_R32G32B32A32_UINT;
                        }
                    }
                }else{
                    if(this->attrTpSize.at(attrIdx)==sizeof(int8_t)){
                        if(this->attrDims.at(attrIdx)==1){
                            return VK_FORMAT_R8_SINT;
                        }else if(this->attrDims.at(attrIdx)==2){
                            return VK_FORMAT_R8G8_SINT;
                        }else if(this->attrDims.at(attrIdx)==3){
                            return VK_FORMAT_R8G8B8_SINT;
                        }else if(this->attrDims.at(attrIdx)==4){
                            return VK_FORMAT_R8G8B8A8_SINT;
                        }
                    }else if(this->attrTpSize.at(attrIdx)==sizeof(int16_t)){
                        if(this->attrDims.at(attrIdx)==1){
                            return VK_FORMAT_R16_SINT;
                        }else if(this->attrDims.at(attrIdx)==2){
                            return VK_FORMAT_R16G16_SINT;
                        }else if(this->attrDims.at(attrIdx)==3){
                            return VK_FORMAT_R16G16B16_SINT;
                        }else if(this->attrDims.at(attrIdx)==4){
                            return VK_FORMAT_R16G16B16A16_SINT;
                        }
                    }else if(this->attrTpSize.at(attrIdx)==sizeof(int32_t)){
                        if(this->attrDims.at(attrIdx)==1){
                            return VK_FORMAT_R32_SINT;
                        }else if(this->attrDims.at(attrIdx)==2){
                            return VK_FORMAT_R32G32_SINT;
                        }else if(this->attrDims.at(attrIdx)==3){
                            return VK_FORMAT_R32G32B32_SINT;
                        }else if(this->attrDims.at(attrIdx)==4){
                            return VK_FORMAT_R32G32B32A32_SINT;
                        }
                    }
                }
            }
            return VK_FORMAT_UNDEFINED;
        }
        bool getInputAttrDescriptionInternal(std::vector<VkVertexInputAttributeDescription>* desc) override{
            ANTH_ASSERT(desc,"Description is nullptr");
            uint32_t offset = 0;
            desc->clear();
            for(auto i=0;i<sizeof...(AttrSz);i++){
                VkVertexInputAttributeDescription attrDesc = {};
                attrDesc.binding = 0;
                attrDesc.location = i;
                attrDesc.format = this->getFormatFromTypeInfo(i);
                attrDesc.offset = offset;
                desc->push_back(attrDesc);
                offset+=this->attrTpSize.at(i)*this->attrDims.at(i);
            }
            return true;
        }
        uint32_t calculateBufferSize() override{
            return this->singleVertexSize*this->totalVertices;
        }

    };

}