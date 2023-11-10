#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "./buffer/AnthemUniformBuffer.h"
#include "./buffer/AnthemShaderStorageBuffer.h"
#include "image/AnthemImage.h"

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

    struct AnthemSamplerDescriptorInfo{
        AnthemImageContainer* img;
        uint32_t bindLoc;
        VkDescriptorSetLayoutBinding layoutBindingDesc = {};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        uint32_t descPoolId;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkDescriptorImageInfo imageInfo = {};
    };

    struct AnthemShaderStorageBufferDescriptorInfo{
        const std::vector<AnthemGeneralBufferProp>* buffer;
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
        std::vector<VkDescriptorSet> descriptorSetsUniform = {};
        std::vector<VkDescriptorSet> descriptorSetsSsbo = {};
        std::vector<VkDescriptorSet> descriptorSetsImg = {};
        std::vector<VkDescriptorPool> descriptorPoolList = {};
        
        std::vector<AnthemShaderStorageBufferDescriptorInfo> ssboDesc;
        std::vector<AnthemUniformBufferDescriptorInfo> uniformBuffers;
        std::vector<AnthemSamplerDescriptorInfo> samplersDesc;

        std::vector<VkWriteDescriptorSet> descWriteSsbo = {};
        std::vector<VkWriteDescriptorSet> descWriteUniform = {};
        std::vector<VkWriteDescriptorSet> descWriteSampler = {};
    public:
        const VkDescriptorSet* getDescriptorSetUniform(uint32_t idx) const{
            return &(descriptorSetsUniform.at(idx));
        }
        const bool appendDescriptorSetUniform(uint32_t idx,std::vector<VkDescriptorSet>* outRef) const{
            outRef->push_back(descriptorSetsUniform.at(idx));
            return true;
        }
        const bool appendDescriptorSetSampler(uint32_t idx,std::vector<VkDescriptorSet>* outRef) const{
            outRef->push_back(descriptorSetsImg.at(idx));
            return true;
        }
        const bool appendDescriptorSetSsbo(uint32_t idx,std::vector<VkDescriptorSet>* outRef) const{
            outRef->push_back(descriptorSetsSsbo.at(idx));
            return true;
        }

        const bool getAllDescriptorSets(uint32_t frameIdx, std::vector<VkDescriptorSet>* outRef){
            for(int i=0;i<this->uniformBuffers.size();i++){
                outRef->push_back(this->descriptorSetsUniform.at(frameIdx*this->uniformBuffers.size()+i));
            }
            for(int i=0;i<this->samplersDesc.size();i++){
                outRef->push_back(this->descriptorSetsImg.at(frameIdx*this->samplersDesc.size()+i));
            }
            for(int i=0;i<this->ssboDesc.size();i++){
                outRef->push_back(this->descriptorSetsSsbo.at(frameIdx*this->ssboDesc.size()+i));
            }
            return true;
        }
        bool addShaderStorageBuffer(AnthemShaderStorageBuffer* ssbo,uint32_t bindLoc, uint32_t descPoolId){
            this->ssboDesc.push_back({});
            auto& layoutBindingDesc = this->ssboDesc.back().layoutBindingDesc;
            auto& layoutCreateInfo = this->ssboDesc.back().layoutCreateInfo;

            this->ssboDesc.back().bindLoc = bindLoc;
            this->ssboDesc.back().descPoolId = descPoolId;
            this->ssboDesc.back().buffer = ssbo->getBuffers();
            this->ssboDesc.back().size = ssbo->getBufferSize();

            layoutBindingDesc.binding = bindLoc;
            layoutBindingDesc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBindingDesc.descriptorCount = 1;
            layoutBindingDesc.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            layoutBindingDesc.pImmutableSamplers = nullptr;

            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = 1;
            layoutCreateInfo.pBindings = &layoutBindingDesc;

            ANTH_LOGI("Start Create Layout");
            if(vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(),&layoutCreateInfo,nullptr,&(this->ssboDesc.back().layout))!=VK_SUCCESS){
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }

        }
        bool addSampler(AnthemImageContainer* imageSampler,uint32_t bindLoc, uint32_t descPoolId){
            this->samplersDesc.push_back({});
            ANTH_LOGI("Spec Binding Addrs", (long long)(this),"/",this->samplersDesc.size());
            auto& layoutBindingDesc = this->samplersDesc.back().layoutBindingDesc;
            auto& layoutCreateInfo = this->samplersDesc.back().layoutCreateInfo;

            this->samplersDesc.back().img = imageSampler;
            this->samplersDesc.back().bindLoc = bindLoc;
            this->samplersDesc.back().descPoolId = descPoolId;

            layoutBindingDesc.binding = bindLoc;
            layoutBindingDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBindingDesc.descriptorCount = 1;
            layoutBindingDesc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layoutBindingDesc.pImmutableSamplers = nullptr;

            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = 1;
            layoutCreateInfo.pBindings = &layoutBindingDesc;

            ANTH_LOGI("Start Create Layout");
            if(vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(),&layoutCreateInfo,nullptr,&(this->samplersDesc.back().layout))!=VK_SUCCESS){
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }
            return true;
        }
        bool addUniformBuffer(AnthemUniformBuffer* uniformBuffer, uint32_t bindLoc, uint32_t descPoolId){
            this->uniformBuffers.push_back({});
            ANTH_LOGI("Spec Binding Addrs");
            auto& layoutBindingDesc = this->uniformBuffers.back().layoutBindingDesc;
            auto& layoutCreateInfo = this->uniformBuffers.back().layoutCreateInfo;

            ANTH_LOGI("Get Buffer Attrs");
            this->uniformBuffers.back().bindLoc = bindLoc;
            this->uniformBuffers.back().buffer = uniformBuffer->getBuffers();
            this->uniformBuffers.back().size = static_cast<uint32_t>(uniformBuffer->getBufferSize());
            this->uniformBuffers.back().descPoolId = descPoolId;

            ANTH_LOGI("Start Spec Info");
            //Create Layout Binding Desc
            layoutBindingDesc.binding = bindLoc;
            layoutBindingDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBindingDesc.descriptorCount = 1;
            layoutBindingDesc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
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
            for(int i=0;i<this->samplersDesc.size();i++){
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(),this->samplersDesc.at(i).layout,nullptr);
            }
            for(int i=0;i<this->ssboDesc.size();i++){
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(),this->ssboDesc.at(i).layout,nullptr);
            }
            return true;
        }

        bool appendUniformDescriptorLayoutIdx(std::vector<VkDescriptorSetLayout>* outRef,int index){
            outRef->push_back(this->uniformBuffers.at(index).layout);
            return true;
        }
        bool appendSamplerDescriptorLayoutIdx(std::vector<VkDescriptorSetLayout>* outRef,int index){
            ANTH_LOGI("Added sampler layout:",this->samplersDesc.at(index).layout);
            outRef->push_back(this->samplersDesc.at(index).layout);
            return true;
        }
        bool appendSsboDescriptorLayoutIdx(std::vector<VkDescriptorSetLayout>* outRef,int index){
            ANTH_LOGI("Added SSBO layout:",this->ssboDesc.at(index).layout);
            outRef->push_back(this->ssboDesc.at(index).layout);
            return true;
        }

        bool getAllDescriptorLayouts(std::vector<VkDescriptorSetLayout>* outRef){
            ANTH_LOGI("Total Size=",this->uniformBuffers.size(),"+",this->samplersDesc.size());
            for(int i=0;i<this->uniformBuffers.size();i++){
                outRef->push_back(this->uniformBuffers.at(i).layout);
            }
            for(int i=0;i<this->samplersDesc.size();i++){
                outRef->push_back(this->samplersDesc.at(i).layout);
            }
            for(int i=0;i<this->ssboDesc.size();i++){
                outRef->push_back(this->ssboDesc.at(i).layout);
            }
            return true;
        }
        const VkDescriptorSetLayout* getDescriptorSetLayoutUniformBuffer(uint32_t idx) const{
            ANTH_LOGI("Total Size=",this->uniformBuffers.size());
            ANTH_LOGI("VisIdx=",idx);
            return &(this->uniformBuffers.at(idx).layout);
        }
        bool createDescriptorSetSsbo(uint32_t numSets){
            ANTH_LOGI("Allocating Descriptor Update: SSBO");
            for(int i=0;i<this->ssboDesc.size();i++){
                ANTH_LOGI("Allocate in Pool:",this->ssboDesc.at(i).descPoolId);
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets,this->ssboDesc.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->ssboDesc.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSetsSsbo.resize(numSets);
                if(vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(),&allocInfo,descriptorSetsSsbo.data())!=VK_SUCCESS){
                    ANTH_LOGE("Failed to allocate descriptor sets");
                    return false;
                }
            }
            ANTH_LOGI("Preparing Descriptor Update: SSBO");
            for(uint32_t i=0;i<numSets;i++){
                for(uint32_t j=0;j<static_cast<uint32_t>(this->ssboDesc.size());j++){
                    ANTH_LOGI("Buffer Copies =", ssboDesc.at(j).buffer->size());
                    ssboDesc.at(j).bufferInfo.buffer = ssboDesc.at(j).buffer->at(i).buffer;
                    ssboDesc.at(j).bufferInfo.offset = 0;
                    ssboDesc.at(j).bufferInfo.range = ssboDesc.at(j).size;

                    VkWriteDescriptorSet descCp = {};
                    descCp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descCp.pNext = nullptr;
                    descCp.dstSet = descriptorSetsSsbo.at(i);
                    descCp.dstBinding = ssboDesc.at(j).bindLoc;
                    descCp.dstArrayElement = 0;
                    descCp.descriptorCount = 1;
                    descCp.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descCp.pImageInfo = nullptr;
                    descCp.pBufferInfo = &ssboDesc.at(j).bufferInfo;
                    descCp.pTexelBufferView = nullptr;

                    descWriteSsbo.push_back(descCp);
                }
                
                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(), static_cast<uint32_t>(descWriteSsbo.size()),descWriteSsbo.data(),0,nullptr);
            }

            return true;
        }
        bool createDescriptorSet(uint32_t numSets){
            createDescriptorSetSsbo(numSets);
            //Create Uniform Descriptor Sets
            ANTH_LOGI("Allocating Descriptor Update: Uniforms");
            for(int i=0;i<this->uniformBuffers.size();i++){
                ANTH_LOGI("Allocate in Pool:",this->uniformBuffers.at(i).descPoolId);
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets,this->uniformBuffers.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->uniformBuffers.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSetsUniform.resize(numSets);
                if(vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(),&allocInfo,descriptorSetsUniform.data())!=VK_SUCCESS){
                    ANTH_LOGE("Failed to allocate descriptor sets");
                    return false;
                }
            }
            ANTH_LOGI("Preparing Descriptor Update: Uniforms");
            for(uint32_t i=0;i<numSets;i++){
                for(uint32_t j=0;j<static_cast<uint32_t>(this->uniformBuffers.size());j++){
                    ANTH_LOGI("Buffer Copies =",uniformBuffers.at(j).buffer->size());
                    uniformBuffers.at(j).bufferInfo.buffer = uniformBuffers.at(j).buffer->at(i).buffer;
                    uniformBuffers.at(j).bufferInfo.offset = 0;
                    uniformBuffers.at(j).bufferInfo.range = uniformBuffers.at(j).size;

                    VkWriteDescriptorSet descCp = {};
                    descCp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descCp.pNext = nullptr;
                    descCp.dstSet = descriptorSetsUniform.at(i);
                    descCp.dstBinding = uniformBuffers.at(j).bindLoc;
                    descCp.dstArrayElement = 0;
                    descCp.descriptorCount = 1;
                    descCp.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descCp.pImageInfo = nullptr;
                    descCp.pBufferInfo = &uniformBuffers.at(j).bufferInfo;
                    descCp.pTexelBufferView = nullptr;

                    descWriteUniform.push_back(descCp);
                }
                
                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(), static_cast<uint32_t>(descWriteUniform.size()),descWriteUniform.data(),0,nullptr);
            }
            //Create Image Descriptor Sets
            ANTH_LOGI("Allocating Descriptor Alloc: Samplers",this->samplersDesc.size());
            for(int i=0;i<this->samplersDesc.size();i++){
                ANTH_LOGI("Allocate in Pool:",this->samplersDesc.at(i).descPoolId);
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets,this->samplersDesc.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->samplersDesc.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSetsImg.resize(numSets);
                auto res = vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(),&allocInfo,descriptorSetsImg.data());
                if(res!=VK_SUCCESS){
                    ANTH_LOGE("Failed to allocate descriptor sets",res);
                    return false;
                }
            }
            ANTH_LOGI("Preparing Descriptor Update: Samplers");
            for(uint32_t i=0;i<numSets;i++){
                for(uint32_t j=0;j<static_cast<uint32_t>(this->samplersDesc.size());j++){
                    samplersDesc.at(j).imageInfo.imageLayout = samplersDesc.at(j).img->getDesiredLayout();
                    samplersDesc.at(j).imageInfo.imageView = *(samplersDesc.at(j).img->getImageView());
                    samplersDesc.at(j).imageInfo.sampler = *(samplersDesc.at(j).img->getSampler());

                    VkWriteDescriptorSet descCp = {};
                    descCp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descCp.pNext = nullptr;
                    descCp.dstSet = descriptorSetsImg.at(i);
                    descCp.dstBinding = samplersDesc.at(j).bindLoc;
                    descCp.dstArrayElement = 0;
                    descCp.descriptorCount = 1;
                    descCp.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descCp.pImageInfo = &samplersDesc.at(j).imageInfo;
                    descCp.pBufferInfo = nullptr;
                    descCp.pTexelBufferView = nullptr;

                    descWriteSampler.push_back(descCp);
                }
                
                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(), static_cast<uint32_t>(descWriteSampler.size()),descWriteSampler.data(),0,nullptr);
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
        bool destroyImageBufferDescriptorLayouts(){
            for(int i=0;i<this->samplersDesc.size();i++){
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(),this->samplersDesc.at(i).layout,nullptr);
            }
            return true;
        }
        bool destroySsboDescriptorLayouts(){
            for(int i=0;i<this->ssboDesc.size();i++){
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(),this->ssboDesc.at(i).layout,nullptr);
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
            *index = static_cast<uint32_t>(this->descriptorPoolList.size())-1;

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
