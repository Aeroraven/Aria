#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "./buffer/AnthemUniformBuffer.h"
#include "./buffer/AnthemShaderStorageBuffer.h"
#include "image/AnthemImage.h"

#include "./descriptor/AnthemSamplerDescriptor.h"
#include "./descriptor/AnthemStorageBufferDescriptor.h"
#include "./descriptor/AnthemUniformBufferDescriptor.h"
#include "./descriptor/AnthemStorageImageDescriptor.h"
#include "./descriptor/AnthemAccStructDescriptor.h"

namespace Anthem::Core{

    class AnthemDescriptorPool:public virtual Util::AnthemUtlLogicalDeviceReqBase,
    public virtual AnthemSamplerDescriptor, public virtual AnthemUniformBufferDescriptor,
    public virtual AnthemStorageBufferDescriptor,public virtual AnthemStorageImageDescriptor,
    public virtual AnthemAccStructDescriptor {

    protected:
        std::vector<VkDescriptorPool> descriptorPoolList = {};
    public:
        AnthemDescriptorPool():AnthemSamplerDescriptor(nullptr),AnthemUniformBufferDescriptor(nullptr),
        AnthemStorageBufferDescriptor(nullptr),AnthemStorageImageDescriptor(nullptr),
        AnthemAccStructDescriptor(nullptr){

        }
        bool virtual specifyLogicalDevice(const AnthemLogicalDevice* device) {
            Util::AnthemUtlLogicalDeviceReqBase::specifyLogicalDevice(device);
            this->setLogicalDeviceSampler(device);
            this->setLogicalDeviceUniform(device);
            this->setLogicalDeviceSsbo(device);
            this->setLogicalDeviceStorageImage(device);
            this->setLogicalDeviceAccStruct(device);
            return true;
        }

        const bool getAllDescriptorSets(uint32_t frameIdx, std::vector<VkDescriptorSet>* outRef){
            getAllDescriptorSetsUniform(frameIdx, outRef);
            getAllDescriptorSetsSampler(frameIdx, outRef);
            getAllDescriptorSetsSsbo(frameIdx, outRef);
            getAllDescriptorSetsStorageImage(frameIdx, outRef);
            getAllDescriptorSetsAccStruct(frameIdx, outRef);
            return true;
        }
        
        bool destroyLayoutBinding(){
            destroyLayoutBindingUniform();
            destroyLayoutBindingSampler();
            destroyLayoutBindingSsbo();
            destroyLayoutBindingStorageImage();
            destroyLayoutBindingAccStruct();
            return true;
        }

        bool getAllDescriptorLayouts(std::vector<VkDescriptorSetLayout>* outRef){
            getAllDescriptorLayoutsUniform(outRef);
            getAllDescriptorLayoutsSampler(outRef);
            getAllDescriptorLayoutsSsbo(outRef);
            getAllDescriptorLayoutsStorageImage(outRef);
            getAllDescriptorLayoutsAccStruct(outRef);
            return true;
        }
        
        bool createDescriptorSet(uint32_t numSets){
            createDescriptorSetSsbo(numSets, descriptorPoolList);
            createDescriptorSetUniform(numSets, descriptorPoolList);
            createDescriptorSetSampler(numSets, descriptorPoolList);
            createDescriptorSetStorageImage(numSets, descriptorPoolList);
            createDescriptorSetAS(numSets, descriptorPoolList);
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

            if(vkCreateDescriptorPool(this->getLogicalDevice()->getLogicalDevice(), &poolCreateInfo, nullptr, &pool) != VK_SUCCESS) {
                ANTH_LOGE("Failed to create descriptor pool");
                return false;
            }
            this->descriptorPoolList.push_back(pool);
            //TODO: Ambiguous Code
            *index = static_cast<uint32_t>(this->descriptorPoolList.size())-1;
            return true;
        }
        bool destroyDescriptorPool(){
            for(int i=0;i<this->descriptorPoolList.size();i++){
                vkDestroyDescriptorPool(this->getLogicalDevice()->getLogicalDevice(),this->descriptorPoolList.at(i),nullptr);
            }
            return true;
        }
    };
}
