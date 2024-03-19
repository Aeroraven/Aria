#pragma once
#include "../../../../include/core/pipeline/raytracing/AnthemRayTracingPipeline.h"

namespace Anthem::Core {
    bool AnthemRayTracingPipeline::specifyShaderModule(const AnthemRayTracingShaders* shaderModule) {
        this->shaderModule = shaderModule;
        return true;
    }
    bool AnthemRayTracingPipeline::destroyPipeline() {
        for (auto& p : this->bindingTableBuffer) {
            this->destroyBufferInternalUt(this->logicalDevice, &p);
        }
        vkDestroyPipeline(this->logicalDevice->getLogicalDevice(), this->pipeline, nullptr);
        this->pipelineCreated = false;
        return true;
    }
    bool AnthemRayTracingPipeline::destroyPipelineLayout() {
        vkDestroyPipelineLayout(this->logicalDevice->getLogicalDevice(), this->pipelineLayout, nullptr);
        return true;
    }
    const VkPipeline* AnthemRayTracingPipeline::getPipeline() const {
        ANTH_ASSERT(this->pipelineCreated, "Invalid pipeline");
        return &(this->pipeline);
    }
     
    bool AnthemRayTracingPipeline::createPipeline() {
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.layout = this->pipelineLayout;
        pipelineCreateInfo.stageCount = this->shaderModule->getShaderStages().size();
        pipelineCreateInfo.pStages = this->shaderModule->getShaderStages().data();
        pipelineCreateInfo.groupCount = this->shaderModule->getShaderGroups().size();
        pipelineCreateInfo.pGroups = this->shaderModule->getShaderGroups().data();
        pipelineCreateInfo.layout = this->pipelineLayout;
        pipelineCreateInfo.maxPipelineRayRecursionDepth = rayRecursion;

        auto res = this->logicalDevice->vkCall_vkCreateRayTracingPipelinesKHR(
            this->logicalDevice->getLogicalDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
            &pipelineCreateInfo, nullptr, &pipeline);
        
        if (res != VK_SUCCESS) {
            ANTH_LOGE("Failed to create compute pipeline:",res);
        }
        ANTH_LOGI("Raytracing pipeline created");
        pipelineCreated = true;
        return true;
    }
    bool  AnthemRayTracingPipeline::setRayRecursion(uint32_t rayRec) {
        this->rayRecursion = rayRec;
        return true;
    }
    bool AnthemRayTracingPipeline::createBindingTable() {
        auto shaderGroups = shaderModule->getShaderGroups();
        auto props = this->getPhyDevice()->getDeivceRaytracingProperties();
        this->handleSize = props.shaderGroupHandleSize;
        this->handleSizeAligned = AT_ALIGN(handleSize, props.shaderGroupHandleAlignment);
        auto totalSize = shaderGroups.size() * this->handleSizeAligned;

        std::vector<uint8_t> shaderHandleStorage(totalSize);
        auto result = this->logicalDevice->vkCall_vkGetRayTracingShaderGroupHandlesKHR(
            this->logicalDevice->getLogicalDevice(), pipeline, 0,
            shaderGroups.size(), totalSize, shaderHandleStorage.data());
        ANTH_ASSERT(result == VK_SUCCESS, "Failed to create SBT");
        const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        for (auto i : AT_RANGE2(shaderGroups.size())) {
            AnthemGeneralBufferProp prop;
            this->createBufferInternalUt(this->logicalDevice, this->phyDevice, &prop, bufferUsageFlags, memoryUsageFlags, nullptr, this->handleSize);
            this->copyDataToBufferInternalUt(this->logicalDevice, &prop,
                shaderHandleStorage.data() + i * this->handleSizeAligned, this->handleSize, true);
            this->bindingTableBuffer.push_back(prop);
        }
        return true;
    }
    const VkPipelineLayout* AnthemRayTracingPipeline::getPipelineLayout() const {
        ANTH_ASSERT(this->pipelineCreated, "Invalid pipeline");
        return &(this->pipelineLayout);
    }
    std::vector<VkStridedDeviceAddressRegionKHR> AnthemRayTracingPipeline::getTraceRayRegions(int32_t raygenId, int32_t missId, int32_t closeHitId, int32_t callableId) const {
        std::vector< VkStridedDeviceAddressRegionKHR> ret;
        std::vector<int32_t> pv = { raygenId,missId,closeHitId,callableId };
        for (auto& x : pv) {
            VkStridedDeviceAddressRegionKHR region{};
            if (x != -1) {
                region.deviceAddress = getBufferDeviceAddressUt(this->logicalDevice, &this->bindingTableBuffer[x]);
                region.size = this->handleSizeAligned;
                region.stride = this->handleSizeAligned;
            }
            ret.push_back(region);
        }
        return ret;
    }

}