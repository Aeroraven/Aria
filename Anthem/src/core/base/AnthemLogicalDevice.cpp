#include "../../../include/core/base/AnthemLogicalDevice.h"

namespace Anthem::Core{
    AnthemLogicalDevice::AnthemLogicalDevice(){
        
    }
    bool AnthemLogicalDevice::specifyDevice(VkDevice device){
        this->logicalDevice = device;
        return true;
    }
    VkDevice AnthemLogicalDevice::getLogicalDevice() const{
        ANTH_ASSERT(this->logicalDevice != VK_NULL_HANDLE,"Logical device not specified");
        return this->logicalDevice;
    }
    bool AnthemLogicalDevice::destroyLogicalDevice(const VkInstance* instance){
        vkDestroyDevice(this->logicalDevice,nullptr);
        this->specifiedDevice = false;
        return true;
    }
    bool AnthemLogicalDevice::specifyGraphicsQueue(VkQueue queue){
        this->graphicsQueue = queue;
        return true;
    }
    bool AnthemLogicalDevice::specifyComputeQueue(VkQueue queue) {
        this->computeQueue = queue;
        return true;
    }
    VkQueue AnthemLogicalDevice::getGraphicsQueue() const{
        ANTH_ASSERT(this->graphicsQueue != VK_NULL_HANDLE,"Graphics queue not specified");
        return this->graphicsQueue;
    }
    bool AnthemLogicalDevice::specifyPresentQueue(VkQueue queue){
        this->presentQueue = queue;
        return true;
    }
    VkQueue AnthemLogicalDevice::getPresentQueue() const{
        ANTH_ASSERT(this->presentQueue != VK_NULL_HANDLE,"Present queue not specified");
        return this->presentQueue;
    }
    VkQueue AnthemLogicalDevice::getComputeQueue() const {
        ANTH_ASSERT(this->computeQueue != VK_NULL_HANDLE, "Present queue not specified");
        return this->computeQueue;
    }
    bool AnthemLogicalDevice::waitForIdle() const{
        vkDeviceWaitIdle(this->logicalDevice);
        return true;
    }
    bool AnthemLogicalDevice::preparePFNs() {
        this->procCmdDrawMesh = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(this->logicalDevice, "vkCmdDrawMeshTasksEXT"));

#define getSinglePFN(name) this->proc##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(this->logicalDevice, "vk"#name))
#define validateSinglePFN(name) ANTH_ASSERT(this->proc##name != nullptr,"proc"#name" is null pointer");
#define prepareSinglePFN(name) getSinglePFN(name);validateSinglePFN(name);

#ifdef AT_FEATURE_RAYTRACING_ENABLED

        prepareSinglePFN(GetBufferDeviceAddressKHR);
        prepareSinglePFN(CmdBuildAccelerationStructuresKHR);
        prepareSinglePFN(BuildAccelerationStructuresKHR);
        prepareSinglePFN(CreateAccelerationStructureKHR);
        prepareSinglePFN(DestroyAccelerationStructureKHR);
        prepareSinglePFN(GetAccelerationStructureBuildSizesKHR);
        prepareSinglePFN(GetAccelerationStructureDeviceAddressKHR);
        prepareSinglePFN(CmdTraceRaysKHR);
        prepareSinglePFN(GetRayTracingShaderGroupHandlesKHR);
        prepareSinglePFN(CreateRayTracingPipelinesKHR);

#endif

#undef prepareSinglePFN
#undef validateSinglePFN
#undef getSinglePFN

        return true;
    }
    void AnthemLogicalDevice::vkCall_vkCmdDrawMeshTaskExt(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        (*this->procCmdDrawMesh)(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
    VkDeviceAddress AnthemLogicalDevice::vkCall_vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
        return (*this->procGetBufferDeviceAddressKHR)(device, pInfo);
    }
    void AnthemLogicalDevice::vkCall_vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer  commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
        return (*this->procCmdBuildAccelerationStructuresKHR)(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
    }
    VkResult AnthemLogicalDevice::vkCall_vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
        return (*this->procBuildAccelerationStructuresKHR)(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
    }
    VkResult AnthemLogicalDevice::vkCall_vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure) {
        return (*this->procCreateAccelerationStructureKHR)(device, pCreateInfo, pAllocator, pAccelerationStructure);
    }
    void  AnthemLogicalDevice::vkCall_vkDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) {
        return (*this->procDestroyAccelerationStructureKHR)(device, accelerationStructure, pAllocator);
    }
    void AnthemLogicalDevice::vkCall_vkGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR  buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo, const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
        return (*this->procGetAccelerationStructureBuildSizesKHR)(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    }
    VkDeviceAddress AnthemLogicalDevice::vkCall_vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
        return (*this->procGetAccelerationStructureDeviceAddressKHR)(device, pInfo);
    }
    void AnthemLogicalDevice::vkCall_vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) {
        return (*this->procCmdTraceRaysKHR)(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    }
    VkResult AnthemLogicalDevice::vkCall_vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
        return (*this->procGetRayTracingShaderGroupHandlesKHR)(device, pipeline, firstGroup, groupCount, dataSize, pData);
    }
    VkResult AnthemLogicalDevice::vkCall_vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
        return (*this->procCreateRayTracingPipelinesKHR)(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    }
}