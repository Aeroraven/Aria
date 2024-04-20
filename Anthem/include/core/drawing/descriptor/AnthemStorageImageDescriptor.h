#pragma once

#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../buffer/AnthemUniformBuffer.h"
#include "../buffer/AnthemShaderStorageBuffer.h"
#include "../image/AnthemImage.h"

namespace Anthem::Core {
    struct AnthemStorageImageDescriptorInfo {
        std::vector<AnthemImageContainer*> img;
        uint32_t bindLoc;
        VkDescriptorSetLayoutBinding layoutBindingDesc = {};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        uint32_t descPoolId;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorImageInfo> imageInfo = {};
    };

    class AnthemStorageImageDescriptor {
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
    protected:
        std::vector<VkDescriptorSet> descriptorSetsStorageImg = {};
        std::vector<AnthemStorageImageDescriptorInfo> storageImgDesc;
        std::vector<VkWriteDescriptorSet> descWriteStorageImg = {};
    public:
        AnthemStorageImageDescriptor(AnthemLogicalDevice* device) {
            this->logicalDevice = device;
        }
        bool virtual setLogicalDeviceStorageImage(const AnthemLogicalDevice* device) {
            this->logicalDevice = device;
            return true;
        }
        const bool virtual appendDescriptorSetStorageImage(uint32_t idx, std::vector<VkDescriptorSet>* outRef) const {
            outRef->push_back(descriptorSetsStorageImg.at(idx));
            return true;
        }
        bool addStorageImageArray (std::vector<AnthemImageContainer*> imageSampler, uint32_t bindLoc, uint32_t descPoolId) {
            this->storageImgDesc.push_back({});
            auto& layoutBindingDesc = this->storageImgDesc.back().layoutBindingDesc;
            auto& layoutCreateInfo = this->storageImgDesc.back().layoutCreateInfo;
            layoutBindingDesc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            this->storageImgDesc.back().bindLoc = bindLoc;
            this->storageImgDesc.back().descPoolId = descPoolId;
            for (int i = 0; i < imageSampler.size(); i++) {
                this->storageImgDesc.back().img.push_back(imageSampler[i]);
                this->storageImgDesc.back().imageInfo.push_back({});
                layoutBindingDesc.stageFlags |= imageSampler[i]->getRequiredShaderStage();
                layoutBindingDesc.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
            }
            layoutBindingDesc.binding = bindLoc;
            layoutBindingDesc.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            layoutBindingDesc.descriptorCount = imageSampler.size();

            layoutBindingDesc.pImmutableSamplers = nullptr;

            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = 1;
            layoutCreateInfo.pBindings = &layoutBindingDesc;

            ANTH_CHECK_NULL(logicalDevice);
            if (vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(), &layoutCreateInfo, nullptr, &(this->storageImgDesc.back().layout)) != VK_SUCCESS) {
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }
            return true;
        }

        bool createDescriptorSetStorageImage(uint32_t numSets, std::vector<VkDescriptorPool>& descriptorPoolList) {
            for (int i = 0; i < this->storageImgDesc.size(); i++) {
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets, this->storageImgDesc.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->storageImgDesc.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSetsStorageImg.resize(numSets);
                auto res = vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(), &allocInfo, descriptorSetsStorageImg.data());
                if (res != VK_SUCCESS) {
                    ANTH_LOGE("Failed to allocate descriptor sets", res);
                    return false;
                }
            }

            for (uint32_t i = 0; i < numSets; i++) {
                for (uint32_t j = 0; j < static_cast<uint32_t>(this->storageImgDesc.size()); j++) {
                    for (uint32_t k = 0; k < storageImgDesc.at(j).imageInfo.size(); k++) {
                        storageImgDesc.at(j).imageInfo[k].imageLayout = storageImgDesc.at(j).img[k]->getDesiredLayout();
                        storageImgDesc.at(j).imageInfo[k].imageView = *(storageImgDesc.at(j).img[k]->getImageView());
                        storageImgDesc.at(j).imageInfo[k].sampler = VK_NULL_HANDLE;  //*(storageImgDesc.at(j).img[k]->getSampler());
                    }

                    VkWriteDescriptorSet descCp = {};
                    descCp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descCp.pNext = nullptr;
                    descCp.dstSet = descriptorSetsStorageImg.at(i);
                    descCp.dstBinding = storageImgDesc.at(j).bindLoc;
                    descCp.dstArrayElement = 0;
                    descCp.descriptorCount = storageImgDesc.at(j).imageInfo.size();
                    descCp.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descCp.pImageInfo = storageImgDesc.at(j).imageInfo.data();
                    descCp.pBufferInfo = nullptr;
                    descCp.pTexelBufferView = nullptr;

                    descWriteStorageImg.push_back(descCp);
                }
                ANTH_CHECK_NULL(logicalDevice);
                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(), static_cast<uint32_t>(descWriteStorageImg.size()), descWriteStorageImg.data(), 0, nullptr);
            }
            return true;
        }
        bool appendStorageImageDescriptorLayoutIdx(std::vector<VkDescriptorSetLayout>* outRef, int index) {
            outRef->push_back(this->storageImgDesc.at(index).layout);
            return true;
        }
        bool destroyStorageImageBufferDescriptorLayouts() {
            for (int i = 0; i < this->storageImgDesc.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->storageImgDesc.at(i).layout, nullptr);
            }
            return true;
        }
        const bool getAllDescriptorSetsStorageImage(uint32_t frameIdx, std::vector<VkDescriptorSet>* outRef) {
            for (int i = 0; i < this->storageImgDesc.size(); i++) {
                outRef->push_back(this->descriptorSetsStorageImg.at(frameIdx * this->storageImgDesc.size() + i));
            }
            return true;
        }
        bool destroyLayoutBindingStorageImage() {
            for (int i = 0; i < this->storageImgDesc.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->storageImgDesc.at(i).layout, nullptr);
            }
            return true;
        }
        bool getAllDescriptorLayoutsStorageImage(std::vector<VkDescriptorSetLayout>* outRef) {
            for (int i = 0; i < this->storageImgDesc.size(); i++) {
                outRef->push_back(this->storageImgDesc.at(i).layout);
            }
            return true;
        }
    };
}