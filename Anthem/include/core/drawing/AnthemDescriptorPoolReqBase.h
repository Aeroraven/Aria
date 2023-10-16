#pragma once 
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemLogicalDevice.h"
namespace Anthem::Core{
    class AnthemDescriptorPoolReqBase{
    protected:
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets = {};
    public:
        const VkDescriptorSet* getDescriptorSet(uint32_t idx) const{
            return &(descriptorSets.at(idx));
        }
        bool createDescriptorPoolInternal(const AnthemLogicalDevice* logicalDevice, uint32_t numSets,VkDescriptorType usage){
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

            if(vkCreateDescriptorPool(logicalDevice->getLogicalDevice(),&poolCreateInfo,nullptr,&descriptorPool)!=VK_SUCCESS){
                ANTH_LOGE("Failed to create descriptor pool");
                return false;
            }
            return true;
        }
        bool destroyDescriptorPoolInternal(const AnthemLogicalDevice* logicalDevice){
            vkDestroyDescriptorPool(logicalDevice->getLogicalDevice(),descriptorPool,nullptr);
            return true;
        }
    };
}