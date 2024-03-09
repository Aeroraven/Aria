#pragma once

#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../buffer/AnthemUniformBuffer.h"
#include "../buffer/AnthemShaderStorageBuffer.h"
#include "../image/AnthemImage.h"


namespace Anthem::Core {
    struct AnthemUniformBufferDescriptorInfo {
        const std::vector<AnthemUniformBufferProp>* buffer;
        uint32_t size;
        VkDescriptorBufferInfo bufferInfo = {};
        uint32_t bindLoc;
        VkDescriptorSetLayoutBinding layoutBindingDesc = {};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        uint32_t descPoolId;
    };

    class AnthemUniformBufferDescriptor {
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
    protected:
        std::vector<VkDescriptorSet> descriptorSetsUniform = {};
        std::vector<AnthemUniformBufferDescriptorInfo> uniformBuffers;
        std::vector<VkWriteDescriptorSet> descWriteUniform = {};

    public:
        AnthemUniformBufferDescriptor(AnthemLogicalDevice* device) {
            this->logicalDevice = device;
        }
        bool virtual setLogicalDeviceUniform(const AnthemLogicalDevice* device) {
            this->logicalDevice = device;
            return true;
        }
        const VkDescriptorSet* getDescriptorSetUniform(uint32_t idx) const {
            return &(descriptorSetsUniform.at(idx));
        }
        const bool appendDescriptorSetUniform(uint32_t idx, std::vector<VkDescriptorSet>* outRef) const {
            outRef->push_back(descriptorSetsUniform.at(idx));
            return true;
        }
        bool addUniformBuffer(AnthemUniformBuffer* uniformBuffer, uint32_t bindLoc, uint32_t descPoolId) {
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
            if (vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(), &layoutCreateInfo, nullptr, &(this->uniformBuffers.back().layout)) != VK_SUCCESS) {
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }
            return true;
        }
        const VkDescriptorSetLayout* getDescriptorSetLayoutUniformBuffer(uint32_t idx) const {
            return &(this->uniformBuffers.at(idx).layout);
        }

        bool createDescriptorSetUniform(uint32_t numSets, std::vector<VkDescriptorPool>& descriptorPoolList) {
            for (int i = 0; i < this->uniformBuffers.size(); i++) {
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets, this->uniformBuffers.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->uniformBuffers.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSetsUniform.resize(numSets);
                if (vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(), &allocInfo, descriptorSetsUniform.data()) != VK_SUCCESS) {
                    ANTH_LOGE("Failed to allocate descriptor sets");
                    return false;
                }
            }

            for (uint32_t i = 0; i < numSets; i++) {
                for (uint32_t j = 0; j < static_cast<uint32_t>(this->uniformBuffers.size()); j++) {
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

                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(), static_cast<uint32_t>(descWriteUniform.size()), descWriteUniform.data(), 0, nullptr);
            }
        }

        bool destroyUniformBufferDescriptorLayouts() {
            for (int i = 0; i < this->uniformBuffers.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->uniformBuffers.at(i).layout, nullptr);
            }
            return true;
        }
        bool appendUniformDescriptorLayoutIdx(std::vector<VkDescriptorSetLayout>* outRef, int index) {
            outRef->push_back(this->uniformBuffers.at(index).layout);
            return true;
        }
        const bool getAllDescriptorSetsUniform(uint32_t frameIdx, std::vector<VkDescriptorSet>* outRef) {
            for (int i = 0; i < this->uniformBuffers.size(); i++) {
                outRef->push_back(this->descriptorSetsUniform.at(frameIdx * this->uniformBuffers.size() + i));
            }
            return true;
        }
        bool destroyLayoutBindingUniform() {
            for (int i = 0; i < this->uniformBuffers.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->uniformBuffers.at(i).layout, nullptr);
            }
            return true;
        }
        bool getAllDescriptorLayoutsUniform(std::vector<VkDescriptorSetLayout>* outRef) {
            for (int i = 0; i < this->uniformBuffers.size(); i++) {
                outRef->push_back(this->uniformBuffers.at(i).layout);
            }
            return true;
        }
    };
}