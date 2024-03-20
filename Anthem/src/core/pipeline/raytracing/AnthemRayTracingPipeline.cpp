#pragma once
#include "../../../../include/core/pipeline/raytracing/AnthemRayTracingPipeline.h"

namespace Anthem::Core {
    bool AnthemRayTracingPipeline::specifyShaderModule(const AnthemRayTracingShaders* shaderModule) {
        this->shaderModule = shaderModule;
        return true;
    }
    bool AnthemRayTracingPipeline::destroyPipeline() {
        if (bindingTableBuffer.raygenCreated) this->destroyBufferInternalUt(this->logicalDevice, &bindingTableBuffer.raygenSbt);
        if (bindingTableBuffer.hitCreated) this->destroyBufferInternalUt(this->logicalDevice, &bindingTableBuffer.hitSbt);
        if (bindingTableBuffer.missCreated) this->destroyBufferInternalUt(this->logicalDevice, &bindingTableBuffer.missSbt);
        if (bindingTableBuffer.callableCreated) this->destroyBufferInternalUt(this->logicalDevice, &bindingTableBuffer.callableSbt);

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
     
    bool AnthemRayTracingPipeline::createPipeline(){
        auto shaderStages = this->shaderModule->getShaderStages();
        auto shaderGroups = this->shaderModule->getShaderGroups();
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipelineCreateInfo.layout = this->pipelineLayout;
        pipelineCreateInfo.stageCount = shaderStages.size();
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.groupCount = shaderGroups.size();
        pipelineCreateInfo.pGroups = shaderGroups.data();
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
        auto sgSize = shaderModule->getShaderGroupSizes();
        auto props = this->getPhyDevice()->getDeivceRaytracingProperties();
        this->handleSize = props.shaderGroupHandleSize;
        this->handleSizeAligned = AT_ALIGN(handleSize, props.shaderGroupHandleAlignment);
        uint32_t raygenSt = 0, raygenLen = sgSize.raygenSize * handleSize;
        uint32_t missSt = AT_ALIGN(raygenLen, handleSizeAligned), missLen = sgSize.missSize * handleSize;
        uint32_t hitSt = AT_ALIGN(missSt+ missLen, handleSizeAligned), hitLen = sgSize.hitSize * handleSize;
        uint32_t callableSt = AT_ALIGN(hitSt + hitLen, handleSizeAligned), callableLen = sgSize.callableSize * handleSize;
        
        auto totalSize = shaderModule->getShaderGroups().size() * this->handleSizeAligned;
        std::vector<uint8_t> shaderHandleStorage(totalSize);

        auto result = this->logicalDevice->vkCall_vkGetRayTracingShaderGroupHandlesKHR(
            this->logicalDevice->getLogicalDevice(), pipeline, 0,
            shaderModule->getShaderGroups().size(), totalSize, shaderHandleStorage.data());


        ANTH_ASSERT(result == VK_SUCCESS, "Failed to create SBT");
        const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkMemoryAllocateFlagsInfo allocFlags{};
        allocFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        allocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

        auto createSbtBuffer = [&](AnthemGeneralBufferProp* buf,uint32_t handleStart, uint32_t hanleLen,bool& createdFlag) {
            if (hanleLen == 0)return;
            createdFlag = true;
            this->createBufferInternalUt(this->logicalDevice, this->phyDevice, buf, bufferUsageFlags, memoryUsageFlags,
                &allocFlags, hanleLen);
            this->copyDataToBufferInternalUt(this->logicalDevice, buf,
                shaderHandleStorage.data() + handleStart, hanleLen, true);
        };
        createSbtBuffer(&bindingTableBuffer.raygenSbt, raygenSt, raygenLen, bindingTableBuffer.raygenCreated);
        createSbtBuffer(&bindingTableBuffer.missSbt, missSt, missLen, bindingTableBuffer.missCreated);
        createSbtBuffer(&bindingTableBuffer.hitSbt, hitSt, hitLen, bindingTableBuffer.hitCreated);
        createSbtBuffer(&bindingTableBuffer.callableSbt, callableSt, callableLen, bindingTableBuffer.callableCreated);

        return true;
    }
    const VkPipelineLayout* AnthemRayTracingPipeline::getPipelineLayout() const {
        ANTH_ASSERT(this->pipelineCreated, "Invalid pipeline");
        return &(this->pipelineLayout);
    }
    std::vector<VkStridedDeviceAddressRegionKHR> AnthemRayTracingPipeline::getTraceRayRegions() const {
        std::vector< VkStridedDeviceAddressRegionKHR> ret;
        auto sgSize = shaderModule->getShaderGroupSizes();
        std::vector<uint32_t> pv = { sgSize.raygenSize,sgSize.missSize,sgSize.hitSize,sgSize.callableSize};
        std::vector<const AnthemGeneralBufferProp*> bf = { &bindingTableBuffer.raygenSbt,&bindingTableBuffer.missSbt,&bindingTableBuffer.hitSbt,
            &bindingTableBuffer.callableSbt};

        for (auto i : AT_RANGE2(pv.size())) {
            VkStridedDeviceAddressRegionKHR region{};
            if (pv[i] != 0) {
                region.deviceAddress = getBufferDeviceAddressUt(this->logicalDevice, bf[i]);
                region.size = pv[i] * this->handleSize;
                region.stride = this->handleSize;
            }
            ret.push_back(region);
        }
        return ret;
    }

}