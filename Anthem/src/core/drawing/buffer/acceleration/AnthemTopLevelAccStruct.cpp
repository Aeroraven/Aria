#include "../../../../../include/core/drawing/buffer/acceleration/AnthemTopLevelAccStruct.h"

namespace Anthem::Core {
	bool AnthemTopLevelAccStruct::buildTLAS() {
		VkAccelerationStructureBuildGeometryInfoKHR asBuildGeoInfo{};
		std::vector<VkAccelerationStructureGeometryKHR> structGeos;
		std::vector<uint32_t> primitiveCnts;
		for (auto& p : instances) {
			structGeos.push_back(p->getGeometry());
			primitiveCnts.push_back(p->getPrimitiveCounts());
		}
		asBuildGeoInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		asBuildGeoInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		asBuildGeoInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		asBuildGeoInfo.geometryCount = structGeos.size();
		asBuildGeoInfo.pGeometries = structGeos.data();

		VkAccelerationStructureBuildSizesInfoKHR asBuildSize{};
		asBuildSize.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			logicalDevice->getLogicalDevice(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&asBuildGeoInfo,
			primitiveCnts.data(),
			&asBuildSize);
		this->createAccelerationStructBuffer(asBuildSize.accelerationStructureSize);

		VkAccelerationStructureCreateInfoKHR asCreateInfo{};
		asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCreateInfo.buffer = this->asBuffer.buffer,
		asCreateInfo.size = asBuildSize.accelerationStructureSize;
		asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

		this->logicalDevice->vkCall_vkCreateAccelerationStructureKHR(
			this->logicalDevice->getLogicalDevice(),
			&asCreateInfo, nullptr, &this->asHandle
		);
		this->createScratchBuffer(asBuildSize.buildScratchSize);

		VkAccelerationStructureBuildGeometryInfoKHR asBuildGeoInfo2{};
		asBuildGeoInfo2.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		asBuildGeoInfo2.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		asBuildGeoInfo2.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		asBuildGeoInfo2.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		asBuildGeoInfo2.dstAccelerationStructure = this->asHandle;
		asBuildGeoInfo2.geometryCount = structGeos.size();
		asBuildGeoInfo2.pGeometries = structGeos.data();
		asBuildGeoInfo2.scratchData.deviceAddress = scratchDevice.deviceAddress;

		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> asBuildRanges;
		for (auto x : primitiveCnts) {
			VkAccelerationStructureBuildRangeInfoKHR* asBuildRange = new VkAccelerationStructureBuildRangeInfoKHR();
			asBuildRange->primitiveCount = x;
			asBuildRange->primitiveOffset = 0;
			asBuildRange->firstVertex = 0;
			asBuildRange->transformOffset = 0;
			asBuildRanges.push_back(asBuildRange);
		}
		uint32_t cmdBuf;
		this->cmdBufs->createCommandBuffer(&cmdBuf);
		this->logicalDevice->vkCall_vkCmdBuildAccelerationStructuresKHR(
			*this->cmdBufs->getCommandBuffer(cmdBuf),
			1, &asBuildGeoInfo2, asBuildRanges.data()
		);
		this->cmdBufs->submitTaskToGraphicsQueue(cmdBuf, true);

		VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddrInfo{};
		asDeviceAddrInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		asDeviceAddrInfo.accelerationStructure = this->asHandle;
		this->asDeviceAddr.deviceAddress = this->logicalDevice->vkCall_vkGetAccelerationStructureDeviceAddressKHR(
			this->logicalDevice->getLogicalDevice(),
			&asDeviceAddrInfo
		);
		this->destroyBufferInternal(&this->scratchBuffer);
		return true;

	}
}