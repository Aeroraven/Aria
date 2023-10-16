#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "AnthemGeneralBufferBase.h"

namespace Anthem::Core{

    struct AnthemUniformBufferProp:public AnthemGeneralBufferProp{
        void* mappedMem;
    };

    class AnthemUniformBuffer:public AnthemGeneralBufferBase{
    private:
        VkDescriptorSetLayoutBinding layoutBindingDesc = {};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

        std::vector<AnthemUniformBufferProp> uniformBuffers = {};
        std::vector<VkDescriptorSet> descriptorSets = {};

    protected:
        size_t virtual getBufferSize() = 0;
        bool virtual getRawBufferData(void** ptr) = 0;

    protected:
        uint32_t calculateBufferSize() override{
            return this->getBufferSize();
        }

    public:
        const VkDescriptorSet* getDescriptorSet(uint32_t idx) const{
            return &(descriptorSets.at(idx));
        }
        const VkDescriptorSetLayout* getDescriptorSetLayout() const{
            return &layout;
        }
        bool createDescriptorPool(uint32_t numSets){
            VkDescriptorPoolSize poolSize = {};
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSize.descriptorCount = numSets;

            VkDescriptorPoolCreateInfo poolCreateInfo = {};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.pNext = nullptr;
            poolCreateInfo.flags = 0;
            poolCreateInfo.maxSets = numSets;
            poolCreateInfo.poolSizeCount = 1;
            poolCreateInfo.pPoolSizes = &poolSize;

            if(vkCreateDescriptorPool(this->logicalDevice->getLogicalDevice(),&poolCreateInfo,nullptr,&descriptorPool)!=VK_SUCCESS){
                ANTH_LOGE("Failed to create descriptor pool");
                return false;
            }
            return true;
        }
        bool destroyDescriptorPool(){
            vkDestroyDescriptorPool(this->logicalDevice->getLogicalDevice(),descriptorPool,nullptr);
            return true;
        }
        bool createDescriptorSet(uint32_t numSets){
            //Create
            std::vector<VkDescriptorSetLayout> descSetLayout(numSets,layout);
            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.pNext = nullptr;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = numSets;
            allocInfo.pSetLayouts = descSetLayout.data();

            descriptorSets.resize(numSets);
            if(vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(),&allocInfo,descriptorSets.data())!=VK_SUCCESS){
                ANTH_LOGE("Failed to allocate descriptor sets");
                return false;
            }

            for(int i=0;i<numSets;i++){
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = uniformBuffers.at(i).buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = this->getBufferSize();

                VkWriteDescriptorSet descWrite = {};
                descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descWrite.pNext = nullptr;
                descWrite.dstSet = descriptorSets.at(i);
                descWrite.dstBinding = 0;
                descWrite.dstArrayElement = 0;
                descWrite.descriptorCount = 1;
                descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descWrite.pImageInfo = nullptr;
                descWrite.pBufferInfo = &bufferInfo;
                descWrite.pTexelBufferView = nullptr;

                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(),1,&descWrite,0,nullptr);
            }
            return true;
        }
        bool createBuffer(uint32_t numCopies){
            uniformBuffers.clear();
            for(int i=0;i<numCopies;i++){
                uniformBuffers.push_back({});
                this->createBufferInternal(&(uniformBuffers.at(i)),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vkMapMemory(this->logicalDevice->getLogicalDevice(),uniformBuffers.at(i).bufferMem,0,this->getBufferSize(),0,&(uniformBuffers.at(i).mappedMem));
            }
            return true;
        }
        bool destroyBuffers(){
            for(int i=0;i<uniformBuffers.size();i++){
                vkUnmapMemory(this->logicalDevice->getLogicalDevice(),uniformBuffers.at(i).bufferMem);
                vkFreeMemory(this->logicalDevice->getLogicalDevice(),uniformBuffers.at(i).bufferMem,nullptr);
                vkDestroyBuffer(this->logicalDevice->getLogicalDevice(),uniformBuffers.at(i).buffer,nullptr);
            }
            return true;
        }
        bool updateBuffer(uint32_t idx){
            ANTH_LOGV("Updating Uniforms");
            void* data;
            this->getRawBufferData(&data);
            memcpy(uniformBuffers.at(idx).mappedMem,data,this->getBufferSize());
            return true;
        }

        bool createLayoutBinding(uint32_t bindingLoc){
            //Create Layout Binding Desc
            layoutBindingDesc.binding = bindingLoc;
            layoutBindingDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBindingDesc.descriptorCount = 1;
            layoutBindingDesc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            layoutBindingDesc.pImmutableSamplers = nullptr;

            //Create Layout
            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = 1;
            layoutCreateInfo.pBindings = &layoutBindingDesc;

            if(vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(),&layoutCreateInfo,nullptr,&layout)!=VK_SUCCESS){
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }
            return true;
        }
        bool destroyLayoutBinding(){
            vkDestroyDescriptorSetLayout(logicalDevice->getLogicalDevice(),layout,nullptr);
            return true;
        }
    };
    
    template<typename Tp,uint32_t Rk,uint32_t Sz> 
    requires TmplDefs::ATpIsdValidUniform<Tp,Rk,Sz>
    struct AnthemUBDesc;

    template<typename... AUBTpDesc>
    class AnthemUniformBufferImpl;

    template<uint32_t Rk>
    using AnthemUniformMatf = AnthemUBDesc<float,2,Rk>;

    template<uint32_t Rk>
    using AnthemUniformVecf = AnthemUBDesc<float,1,Rk>;

    template<typename... UniTp, uint32_t... UniRk, uint32_t... UniSz>
    class AnthemUniformBufferImpl<AnthemUBDesc<UniTp,UniRk,UniSz>...>:public AnthemUniformBuffer{
    private:
        std::array<uint32_t,sizeof...(UniRk)> uniRanks = {UniRk...};
        std::array<uint32_t,sizeof...(UniSz)> uniVecSize = {UniSz...};
        std::array<uint32_t,sizeof...(UniTp)> uniTpSizeOf = {sizeof(UniTp)...};
        std::array<bool, sizeof...(UniTp)> uniIsFloat = {std::is_same<UniTp,float>::value ...};
        std::array<bool, sizeof...(UniTp)> uniIsInt = {std::is_same<UniTp,int>::value ...};
        
        std::vector<int> alignOffsets = {};
        std::vector<int> variableSize = {};

        char* rawBuffer = nullptr;

    protected:
       
        size_t getRequiredBufferSize(){
            size_t totalSize = 0;
            return alignOffsets.at(alignOffsets.size()-1)+variableSize.at(variableSize.size()-1);
        }
        bool allocateBuffer(){
            rawBuffer = new char[getRequiredBufferSize()];
            return true;
        }
        bool calculatingOffset(){
            int curOffset = 0;
            for(int i=0;i<sizeof...(UniTp);i++){
                int seqLength = uniVecSize.at(i)*((uniVecSize.at(i)-1)*(uniRanks.at(i)==2)+1)*uniTpSizeOf.at(i);
                int reqAlign = uniTpSizeOf.at(i);
                if(uniVecSize.at(i)==1){ //Scalar
                    reqAlign *= 1;
                }else if(uniVecSize.at(i)==2 && uniRanks.at(i)==1){ //Vec2
                    reqAlign *= 2;
                }else if(uniVecSize.at(i)==3 && uniRanks.at(i)==1){ //Vec3
                    reqAlign *= 4;
                }else if(uniVecSize.at(i)==4 && uniRanks.at(i)==1){ //Vec4
                    reqAlign *= 4;
                }else if(uniVecSize.at(i)==4 && uniRanks.at(i)==2){ //Mat4
                    reqAlign *= 4;
                }else{
                    ANTH_LOGE("Unsupported uniform type");
                    return false;
                }
                int alignedOffset = 0;
                if(curOffset%reqAlign==0){
                    alignedOffset = curOffset;
                }else{
                    alignedOffset = (curOffset-curOffset%reqAlign)+reqAlign;
                }
                alignOffsets.push_back(alignedOffset);
                variableSize.push_back(seqLength);
                curOffset = alignedOffset+seqLength;
            }
            return true;
        }
    protected:
        bool getRawBufferData(void** ptr) override{
            *ptr = rawBuffer;
            return true;
        }
        size_t getBufferSize() override{
            return this->getRequiredBufferSize();
        }
    public:
        AnthemUniformBufferImpl(){
            this->calculatingOffset();
            this->allocateBuffer();
        }
        ~AnthemUniformBufferImpl(){
            delete[] rawBuffer;
        }
        bool specifyUniforms(UniTp*... variables){
            std::array<void*,sizeof...(UniTp)> voidPtrs = {variables...};
            for(int i=0;i<sizeof...(UniTp);i++){
                memcpy(rawBuffer+alignOffsets.at(i),voidPtrs.at(i),variableSize.at(i));
            }
            return true;
        }
        

    };
}