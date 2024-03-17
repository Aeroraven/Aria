#pragma once
#include "AnthemLogger.h"
#include "AnthemDefs.h"

namespace Anthem::Core{
    class AnthemLogicalDevice{
    private:
        VkDevice logicalDevice = VK_NULL_HANDLE;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkQueue computeQueue;
        bool specifiedDevice = false;

        PFN_vkCmdDrawMeshTasksEXT procCmdDrawMesh = nullptr;
        PFN_vkGetBufferDeviceAddressKHR procGetBufferDeviceAddressKHR = nullptr;
        PFN_vkCmdBuildAccelerationStructuresKHR procCmdBuildAccelerationStructuresKHR = nullptr;
        PFN_vkBuildAccelerationStructuresKHR procBuildAccelerationStructuresKHR = nullptr;
        PFN_vkCreateAccelerationStructureKHR procCreateAccelerationStructureKHR = nullptr;
        PFN_vkDestroyAccelerationStructureKHR procDestroyAccelerationStructureKHR = nullptr;
        PFN_vkGetAccelerationStructureBuildSizesKHR procGetAccelerationStructureBuildSizesKHR = nullptr;
        PFN_vkGetAccelerationStructureDeviceAddressKHR procGetAccelerationStructureDeviceAddressKHR = nullptr;
        PFN_vkCmdTraceRaysKHR procCmdTraceRaysKHR = nullptr;
        PFN_vkGetRayTracingShaderGroupHandlesKHR procGetRayTracingShaderGroupHandlesKHR = nullptr;
        PFN_vkCreateRayTracingPipelinesKHR procCreateRayTracingPipelinesKHR = nullptr;

    public:
        AnthemLogicalDevice();
        bool virtual specifyDevice(VkDevice device);
        bool virtual specifyGraphicsQueue(VkQueue queue);
        bool virtual specifyPresentQueue(VkQueue queue);
        bool virtual specifyComputeQueue(VkQueue queue);

        VkDevice virtual getLogicalDevice() const;
        VkQueue virtual getGraphicsQueue() const;
        VkQueue virtual getPresentQueue() const;
        VkQueue virtual getComputeQueue() const;


        bool virtual destroyLogicalDevice(const VkInstance* instance) ;
        bool virtual waitForIdle() const;

        bool virtual preparePFNs();

        void virtual vkCall_vkCmdDrawMeshTaskExt(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        VkDeviceAddress virtual vkCall_vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo);
        void virtual vkCall_vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer  commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);
        VkResult virtual vkCall_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);
        VkResult virtual vkCall_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure);
        void virtual vkCall_vkDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator);
        void virtual vkCall_vkGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR  buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo, const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo);
        VkDeviceAddress virtual vkCall_vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo);
        void virtual vkCall_vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth);
        VkResult virtual vkCall_vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData);
        VkResult virtual vkCall_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
    };

}