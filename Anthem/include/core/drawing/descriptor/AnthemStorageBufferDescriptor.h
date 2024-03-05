#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../buffer/AnthemUniformBuffer.h"
#include "../buffer/AnthemShaderStorageBuffer.h"
#include "../image/AnthemImage.h"

namespace Anthem::Core {
    struct AnthemShaderStorageBufferDescriptorInfo {
        const std::vector<AnthemGeneralBufferProp>* buffer;
        uint32_t size;
        VkDescriptorBufferInfo* bufferInfo = nullptr;
        uint32_t bindLoc;
        VkDescriptorSetLayoutBinding layoutBindingDesc = {};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        uint32_t descPoolId;

    };

    class AnthemStorageBufferDescriptor {
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;

    protected:
        std::vector<VkDescriptorSet> descriptorSetsSsbo = {};
        std::vector<AnthemShaderStorageBufferDescriptorInfo> ssboDesc;
        std::vector<VkWriteDescriptorSet> descWriteSsbo = {};

    public:
        AnthemStorageBufferDescriptor(AnthemLogicalDevice* device) {
            this->logicalDevice = device;
        }        
        bool virtual setLogicalDeviceSsbo(const AnthemLogicalDevice* device) {
            this->logicalDevice = device;
            return true;
        }
        const bool appendDescriptorSetSsbo(uint32_t idx, std::vector<VkDescriptorSet>* outRef) const {
            outRef->push_back(descriptorSetsSsbo.at(idx));
            return true;
        }
        bool addShaderStorageBuffer(AnthemShaderStorageBuffer* ssbo, uint32_t bindLoc, uint32_t descPoolId) {
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
            if (vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(), &layoutCreateInfo, nullptr, &(this->ssboDesc.back().layout)) != VK_SUCCESS) {
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }
            return true;
        }
        bool appendSsboDescriptorLayoutIdx(std::vector<VkDescriptorSetLayout>* outRef, int index) {
            ANTH_LOGI("Added SSBO layout:", this->ssboDesc.at(index).layout);
            outRef->push_back(this->ssboDesc.at(index).layout);
            return true;
        }
        bool createDescriptorSetSsbo(uint32_t numSets, std::vector<VkDescriptorPool>& descriptorPoolList) {
            for (int i = 0; i < this->ssboDesc.size(); i++) {
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets, this->ssboDesc.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->ssboDesc.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSetsSsbo.resize(numSets);
                if (vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(), &allocInfo, descriptorSetsSsbo.data()) != VK_SUCCESS) {
                    ANTH_LOGE("Failed to allocate descriptor sets");
                    return false;
                }
            }
            for (uint32_t j = 0; j < static_cast<uint32_t>(this->ssboDesc.size()); j++) {
                ssboDesc.at(j).bufferInfo = new VkDescriptorBufferInfo[numSets];
            }
            for (uint32_t i = 0; i < numSets; i++) {
                for (uint32_t j = 0; j < static_cast<uint32_t>(this->ssboDesc.size()); j++) {
                    ssboDesc.at(j).bufferInfo[i].buffer = ssboDesc.at(j).buffer->at(i).buffer;
                    ssboDesc.at(j).bufferInfo[i].offset = 0;
                    ssboDesc.at(j).bufferInfo[i].range = ssboDesc.at(j).size;


                    VkWriteDescriptorSet descCp = {};
                    descCp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descCp.pNext = nullptr;
                    descCp.dstSet = descriptorSetsSsbo.at(i);
                    descCp.dstBinding = ssboDesc.at(j).bindLoc;
                    descCp.dstArrayElement = 0;
                    descCp.descriptorCount = 1;
                    descCp.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descCp.pImageInfo = nullptr;
                    descCp.pBufferInfo = &ssboDesc.at(j).bufferInfo[i];
                    descCp.pTexelBufferView = nullptr;

                    descWriteSsbo.push_back(descCp);
                }
            }
            vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(), static_cast<uint32_t>(descWriteSsbo.size()), descWriteSsbo.data(), 0, nullptr);
            return true;
        }
        bool destroySsboDescriptorLayouts() {
            for (int i = 0; i < this->ssboDesc.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->ssboDesc.at(i).layout, nullptr);
            }
            return true;
        }

        const bool getAllDescriptorSetsSsbo(uint32_t frameIdx, std::vector<VkDescriptorSet>* outRef) {
            for (int i = 0; i < this->ssboDesc.size(); i++) {
                outRef->push_back(this->descriptorSetsSsbo.at(frameIdx * this->ssboDesc.size() + i));
            }
            return true;
        }

        bool destroyLayoutBindingSsbo() {
            for (int i = 0; i < this->ssboDesc.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->ssboDesc.at(i).layout, nullptr);
            }
            return true;
        }
        bool getAllDescriptorLayoutsSsbo(std::vector<VkDescriptorSetLayout>* outRef) {
            for (int i = 0; i < this->ssboDesc.size(); i++) {
                outRef->push_back(this->ssboDesc.at(i).layout);
            }
            return true;
        }
    };
}