#pragma once

#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../buffer/AnthemUniformBuffer.h"
#include "../buffer/AnthemShaderStorageBuffer.h"
#include "../image/AnthemImage.h"

namespace Anthem::Core {
    struct AnthemSamplerDescriptorInfo {
        std::vector<AnthemImageContainer*> img;
        uint32_t bindLoc;
        VkDescriptorSetLayoutBinding layoutBindingDesc = {};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        uint32_t descPoolId;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorImageInfo> imageInfo = {};
    };

    class AnthemSamplerDescriptor {
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
    protected:
        std::vector<VkDescriptorSet> descriptorSetsSampler = {};
        std::vector<AnthemSamplerDescriptorInfo> samplersDesc;
        std::vector<VkWriteDescriptorSet> descWriteSampler = {};
    public:
        AnthemSamplerDescriptor(AnthemLogicalDevice* device){
            this->logicalDevice = device;
        }
        bool virtual setLogicalDeviceSampler(const AnthemLogicalDevice* device) {
            this->logicalDevice = device;
            return true;
        }
        const bool virtual appendDescriptorSetSampler(uint32_t idx, std::vector<VkDescriptorSet>* outRef) const {
            outRef->push_back(descriptorSetsSampler.at(idx));
            return true;
        }
        bool addSampler(AnthemImageContainer* imageSampler, uint32_t bindLoc, uint32_t descPoolId) {
            return addSamplerArray({ imageSampler }, bindLoc, descPoolId);
        }
        bool addSamplerArray(std::vector<AnthemImageContainer*> imageSampler, uint32_t bindLoc, uint32_t descPoolId) {
            this->samplersDesc.push_back({});
            auto& layoutBindingDesc = this->samplersDesc.back().layoutBindingDesc;
            auto& layoutCreateInfo = this->samplersDesc.back().layoutCreateInfo;
            layoutBindingDesc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            this->samplersDesc.back().bindLoc = bindLoc;
            this->samplersDesc.back().descPoolId = descPoolId;
            for (int i = 0; i < imageSampler.size(); i++) {
                this->samplersDesc.back().img.push_back(imageSampler[i]);
                this->samplersDesc.back().imageInfo.push_back({});
                layoutBindingDesc.stageFlags |= imageSampler[i]->getRequiredShaderStage();
            }
            layoutBindingDesc.binding = bindLoc;
            layoutBindingDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBindingDesc.descriptorCount = imageSampler.size();

            layoutBindingDesc.pImmutableSamplers = nullptr;

            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = 1;
            layoutCreateInfo.pBindings = &layoutBindingDesc;

            ANTH_LOGI("Start Create Layout");
            ANTH_CHECK_NULL(logicalDevice);
            if (vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(), &layoutCreateInfo, nullptr, &(this->samplersDesc.back().layout)) != VK_SUCCESS) {
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }
            return true;
        }
        bool createDescriptorSetSampler(uint32_t numSets, std::vector<VkDescriptorPool>& descriptorPoolList) {
            for (int i = 0; i < this->samplersDesc.size(); i++) {
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets, this->samplersDesc.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->samplersDesc.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSetsSampler.resize(numSets);
                auto res = vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(), &allocInfo, descriptorSetsSampler.data());
                if (res != VK_SUCCESS) {
                    ANTH_LOGE("Failed to allocate descriptor sets", res);
                    return false;
                }
            }

            for (uint32_t i = 0; i < numSets; i++) {
                for (uint32_t j = 0; j < static_cast<uint32_t>(this->samplersDesc.size()); j++) {
                    for (uint32_t k = 0; k < samplersDesc.at(j).imageInfo.size(); k++) {
                        samplersDesc.at(j).imageInfo[k].imageLayout = samplersDesc.at(j).img[k]->getDesiredLayout();
                        samplersDesc.at(j).imageInfo[k].imageView = *(samplersDesc.at(j).img[k]->getImageView());
                        samplersDesc.at(j).imageInfo[k].sampler = *(samplersDesc.at(j).img[k]->getSampler());
                    }

                    VkWriteDescriptorSet descCp = {};
                    descCp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descCp.pNext = nullptr;
                    descCp.dstSet = descriptorSetsSampler.at(i);
                    descCp.dstBinding = samplersDesc.at(j).bindLoc;
                    descCp.dstArrayElement = 0;
                    descCp.descriptorCount = samplersDesc.at(j).imageInfo.size();
                    descCp.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descCp.pImageInfo = samplersDesc.at(j).imageInfo.data();
                    descCp.pBufferInfo = nullptr;
                    descCp.pTexelBufferView = nullptr;

                    descWriteSampler.push_back(descCp);
                }
                ANTH_CHECK_NULL(logicalDevice);
                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(), static_cast<uint32_t>(descWriteSampler.size()), descWriteSampler.data(), 0, nullptr);
            }
        }

        bool appendSamplerDescriptorLayoutIdx(std::vector<VkDescriptorSetLayout>* outRef, int index) {
            ANTH_LOGI("Added sampler layout:", this->samplersDesc.at(index).layout);
            outRef->push_back(this->samplersDesc.at(index).layout);
            return true;
        }
        bool destroyImageBufferDescriptorLayouts() {
            for (int i = 0; i < this->samplersDesc.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->samplersDesc.at(i).layout, nullptr);
            }
            return true;
        }
        const bool getAllDescriptorSetsSampler(uint32_t frameIdx, std::vector<VkDescriptorSet>* outRef) {
            for (int i = 0; i < this->samplersDesc.size(); i++) {
                outRef->push_back(this->descriptorSetsSampler.at(frameIdx * this->samplersDesc.size() + i));
            }
            return true;
        }
        bool destroyLayoutBindingSampler() {
            for (int i = 0; i < this->samplersDesc.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->samplersDesc.at(i).layout, nullptr);
            }
            return true;
        }
        bool getAllDescriptorLayoutsSampler(std::vector<VkDescriptorSetLayout>* outRef) {
            for (int i = 0; i < this->samplersDesc.size(); i++) {
                outRef->push_back(this->samplersDesc.at(i).layout);
            }
            return true;
        }
    };

}