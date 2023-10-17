#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "./AnthemUniformBuffer.h"

namespace Anthem::Core{
    struct AnthemUniformBufferDescriptorInfo{
        const std::vector<AnthemUniformBufferProp>* buffer;
        uint32_t size;
        VkDescriptorBufferInfo bufferInfo = {};
        uint32_t bindLoc;
        VkDescriptorSetLayoutBinding layoutBindingDesc = {};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        uint32_t descPoolId;
    };

    class AnthemDescriptorPool:public Util::AnthemUtlLogicalDeviceReqBase{
    protected:
        std::vector<VkDescriptorSet> descriptorSets = {};
        std::vector<VkDescriptorPool> descriptorPoolList = {};
        
        std::vector<AnthemUniformBufferDescriptorInfo> uniformBuffers;
        std::vector<VkWriteDescriptorSet> descWrite = {};

    public:
        const VkDescriptorSet* getDescriptorSet(uint32_t idx) const{
            return &(descriptorSets.at(idx));
        }
        bool addUniformBuffer(AnthemUniformBuffer* uniformBuffer, uint32_t bindLoc, uint32_t descPoolId){
            this->uniformBuffers.push_back({});
            ANTH_LOGI("Spec Binding Addrs");
            auto& layoutBindingDesc = this->uniformBuffers.back().layoutBindingDesc;
            auto& layoutCreateInfo = this->uniformBuffers.back().layoutCreateInfo;

            ANTH_LOGI("Get Buffer Attrs");
            this->uniformBuffers.back().bindLoc = bindLoc;
            this->uniformBuffers.back().buffer = uniformBuffer->getBuffers();
            this->uniformBuffers.back().size = uniformBuffer->getBufferSize();
            this->uniformBuffers.back().descPoolId = descPoolId;

            ANTH_LOGI("Start Spec Info");
            //Create Layout Binding Desc
            layoutBindingDesc.binding = bindLoc;
            layoutBindingDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBindingDesc.descriptorCount = 1;
            layoutBindingDesc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            layoutBindingDesc.pImmutableSamplers = nullptr;

            //Create Layout
            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = 1;
            layoutCreateInfo.pBindings = &layoutBindingDesc;

            ANTH_LOGI("Start Create Layout");
            if(vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(),&layoutCreateInfo,nullptr,&(this->uniformBuffers.back().layout))!=VK_SUCCESS){
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }
            return true;
        }
        bool destroyLayoutBinding(){
            for(int i=0;i<this->uniformBuffers.size();i++){
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(),this->uniformBuffers.at(i).layout,nullptr);
            }
            return true;
        }
        const VkDescriptorSetLayout* getDescriptorSetLayoutUniformBuffer(uint32_t idx) const{
            ANTH_LOGI("Total Size=",this->uniformBuffers.size());
            ANTH_LOGI("VisIdx=",idx);
            return &(this->uniformBuffers.at(idx).layout);
        }
        bool createDescriptorSet(uint32_t numSets){
            //Create
            for(int i=0;i<this->uniformBuffers.size();i++){
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets,this->uniformBuffers.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->uniformBuffers.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSets.resize(numSets);
                if(vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(),&allocInfo,descriptorSets.data())!=VK_SUCCESS){
                    ANTH_LOGE("Failed to allocate descriptor sets");
                    return false;
                }
            }
            ANTH_LOGI("Preparing Descriptor Update");
            for(int i=0;i<numSets;i++){
                for(int j=0;j<this->uniformBuffers.size();j++){
                    ANTH_LOGI("Buffer Copies =",uniformBuffers.at(j).buffer->size());
                    uniformBuffers.at(j).bufferInfo.buffer = uniformBuffers.at(j).buffer->at(i).buffer;
                    uniformBuffers.at(j).bufferInfo.offset = 0;
                    uniformBuffers.at(j).bufferInfo.range = uniformBuffers.at(j).size;

                    VkWriteDescriptorSet descCp = {};
                    descCp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descCp.pNext = nullptr;
                    descCp.dstSet = descriptorSets.at(i);
                    descCp.dstBinding = 0;
                    descCp.dstArrayElement = 0;
                    descCp.descriptorCount = 1;
                    descCp.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descCp.pImageInfo = nullptr;
                    descCp.pBufferInfo = &uniformBuffers.at(j).bufferInfo;
                    descCp.pTexelBufferView = nullptr;

                    descWrite.push_back(descCp);
                }
                
                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(),descWrite.size(),descWrite.data(),0,nullptr);
            }
            ANTH_LOGI("done");
            return true;
        }
        bool destroyDescriptorSets(){
            ANTH_DEPRECATED_MSG;
            return true;
        }
        bool destroyUniformBufferDescriptorLayouts(){
            for(int i=0;i<this->uniformBuffers.size();i++){
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(),this->uniformBuffers.at(i).layout,nullptr);
            }
            return true;
        }
        bool createDescriptorPool(uint32_t numSets,VkDescriptorType usage, uint32_t* index){
            VkDescriptorPool pool = VK_NULL_HANDLE;

            VkDescriptorPoolSize poolSize = {};
            poolSize.type = usage;
            poolSize.descriptorCount = numSets;

            VkDescriptorPoolCreateInfo poolCreateInfo = {};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.pNext = nullptr;
            poolCreateInfo.flags = 0;
            poolCreateInfo.maxSets = numSets;
            poolCreateInfo.poolSizeCount = 1;
            poolCreateInfo.pPoolSizes = &poolSize;

            if(vkCreateDescriptorPool(this->logicalDevice->getLogicalDevice(),&poolCreateInfo,nullptr,&pool)!=VK_SUCCESS){
                ANTH_LOGE("Failed to create descriptor pool");
                return false;
            }
            this->descriptorPoolList.push_back(pool);
            *index = this->descriptorPoolList.size()-1;

            return true;
        }
        bool destroyDescriptorPool(){
            for(int i=0;i<this->descriptorPoolList.size();i++){
                vkDestroyDescriptorPool(this->logicalDevice->getLogicalDevice(),this->descriptorPoolList.at(i),nullptr);
            }
            return true;
        }
    };
}
