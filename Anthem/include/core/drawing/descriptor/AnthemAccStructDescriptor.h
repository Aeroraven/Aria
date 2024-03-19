#pragma once

#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../buffer/AnthemUniformBuffer.h"
#include "../buffer/AnthemShaderStorageBuffer.h"
#include "../image/AnthemImage.h"
#include "../buffer/acceleration/AnthemTopLevelAccStruct.h"


namespace Anthem::Core {
    struct AnthemAccStructDescriptorInfo {
        std::vector<AnthemTopLevelAccStruct*> buffer;
        std::vector<uint32_t> size;
        std::vector<VkAccelerationStructureKHR> handles;
        uint32_t bindLoc;
        VkDescriptorSetLayoutBinding layoutBindingDesc = {};
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        uint32_t descPoolId;
    };

    class AnthemAccStructDescriptor {
    private:
        const AnthemLogicalDevice* logicalDevice = nullptr;
    protected:
        std::vector<VkDescriptorSet> descriptorSetsAS = {};
        std::vector<AnthemAccStructDescriptorInfo> accStructs;
        std::vector<VkWriteDescriptorSet> descWriteAS = {};
        std::vector<VkWriteDescriptorSetAccelerationStructureKHR> descWriteASEx = {};
        uint32_t elementCounts = 1;

    public:
        AnthemAccStructDescriptor(AnthemLogicalDevice* device) {
            this->logicalDevice = device;
        }
        bool virtual setLogicalDeviceAccStruct(const AnthemLogicalDevice* device) {
            this->logicalDevice = device;
            return true;
        }
        const VkDescriptorSet* getDescriptorSetAccStruct(uint32_t idx) const {
            return &(descriptorSetsAS.at(idx));
        }
        const bool appendDescriptorSetAccStruct(uint32_t idx, std::vector<VkDescriptorSet>* outRef) const {
            outRef->push_back(descriptorSetsAS.at(idx));
            return true;
        }
        bool addASMultiple(std::vector<AnthemTopLevelAccStruct*> asHandles, uint32_t bindLoc, uint32_t descPoolId) {
            this->accStructs.push_back({});
            auto& layoutBindingDesc = this->accStructs.back().layoutBindingDesc;
            auto& layoutCreateInfo = this->accStructs.back().layoutCreateInfo;

            this->accStructs.back().bindLoc = bindLoc;
            for (auto& x : asHandles) {
                this->accStructs.back().buffer.push_back(x);
                this->accStructs.back().size.push_back(1);
                this->accStructs.back().handles.push_back(x->getHandle());
            }
            this->accStructs.back().descPoolId = descPoolId;

            //Create Layout Binding Desc
            layoutBindingDesc.binding = bindLoc;
            layoutBindingDesc.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
            layoutBindingDesc.descriptorCount = asHandles.size();
            layoutBindingDesc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT |
                VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            layoutBindingDesc.pImmutableSamplers = nullptr;

            //Create Layout
            layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutCreateInfo.bindingCount = 1;
            layoutCreateInfo.pBindings = &layoutBindingDesc;
            this->elementCounts = asHandles.size();

            if (vkCreateDescriptorSetLayout(logicalDevice->getLogicalDevice(), &layoutCreateInfo, nullptr, &(this->accStructs.back().layout)) != VK_SUCCESS) {
                ANTH_LOGE("Failed to create descriptor set layout");
                return false;
            }
            return true;
        }

        bool addAS(AnthemTopLevelAccStruct* asHandle, uint32_t bindLoc, uint32_t descPoolId) {
            return addASMultiple({ asHandle }, bindLoc, descPoolId);
        }

        const VkDescriptorSetLayout* getDescriptorSetLayoutAccStruct(uint32_t idx) const {
            return &(this->accStructs.at(idx).layout);
        }

        bool createDescriptorSetAS(uint32_t numSets, std::vector<VkDescriptorPool>& descriptorPoolList) {
            for (int i = 0; i < this->accStructs.size(); i++) {
                std::vector<VkDescriptorSetLayout> descSetLayout(numSets, this->accStructs.at(i).layout);
                VkDescriptorSetAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.descriptorPool = descriptorPoolList.at(this->accStructs.at(i).descPoolId);
                allocInfo.descriptorSetCount = numSets;
                allocInfo.pSetLayouts = descSetLayout.data();

                descriptorSetsAS.resize(numSets);
                if (vkAllocateDescriptorSets(this->logicalDevice->getLogicalDevice(), &allocInfo, descriptorSetsAS.data()) != VK_SUCCESS) {
                    ANTH_LOGE("Failed to allocate descriptor sets");
                    return false;
                }
            }

            for (uint32_t i = 0; i < numSets; i++) {
                for (uint32_t j = 0; j < static_cast<uint32_t>(this->accStructs.size()); j++) {

                    VkWriteDescriptorSetAccelerationStructureKHR tpx{};
                    tpx.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
                    tpx.accelerationStructureCount = accStructs.at(j).handles.size();
                    tpx.pAccelerationStructures = accStructs.at(j).handles.data();
                    descWriteASEx.push_back(std::move(tpx));

                    VkWriteDescriptorSet descCp = {};
                    descCp.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descCp.pNext = &descWriteASEx[-1];
                    descCp.dstSet = descriptorSetsAS.at(i);
                    descCp.dstBinding = accStructs.at(j).bindLoc;
                    descCp.dstArrayElement = 0;
                    descCp.descriptorCount = this->elementCounts;
                    descCp.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                    descCp.pImageInfo = nullptr;
                    descCp.pBufferInfo = nullptr;
                    descCp.pTexelBufferView = nullptr;

                    descWriteAS.push_back(descCp);
                }

                vkUpdateDescriptorSets(this->logicalDevice->getLogicalDevice(), static_cast<uint32_t>(descWriteAS.size()), descWriteAS.data(), 0, nullptr);
            }
        }

        bool destroyAccStructDescriptorLayouts() {
            for (int i = 0; i < this->accStructs.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->accStructs.at(i).layout, nullptr);
            }
            return true;
        }
        bool appendAccStructDescriptorLayoutIdx(std::vector<VkDescriptorSetLayout>* outRef, int index) {
            outRef->push_back(this->accStructs.at(index).layout);
            return true;
        }
        const bool getAllDescriptorSetsAccStruct(uint32_t frameIdx, std::vector<VkDescriptorSet>* outRef) {
            for (int i = 0; i < this->accStructs.size(); i++) {
                outRef->push_back(this->descriptorSetsAS.at(frameIdx * this->accStructs.size() + i));
            }
            return true;
        }
        bool destroyLayoutBindingAccStruct() {
            for (int i = 0; i < this->accStructs.size(); i++) {
                vkDestroyDescriptorSetLayout(this->logicalDevice->getLogicalDevice(), this->accStructs.at(i).layout, nullptr);
            }
            return true;
        }
        bool getAllDescriptorLayoutsAccStruct(std::vector<VkDescriptorSetLayout>* outRef) {
            for (int i = 0; i < this->accStructs.size(); i++) {
                outRef->push_back(this->accStructs.at(i).layout);
            }
            return true;
        }
    };
}