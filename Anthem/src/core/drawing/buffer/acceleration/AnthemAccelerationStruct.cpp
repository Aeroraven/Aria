#include "../../../../../include/core/drawing/buffer/acceleration/AnthemAccelerationStruct.h"

namespace Anthem::Core {
	bool AnthemAccelerationStruct::destroyBuffer() {
		return this->destroyBufferInternal(&this->asBuffer);;
	}
	bool AnthemAccelerationStruct::createAccelerationStructBuffer(uint32_t bufferSize) {
		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		return this->createBufferInternal(
			&this->asBuffer, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateFlagsInfo, bufferSize);
	}

	bool AnthemAccelerationStruct::createScratchBuffer(uint32_t bufferSize) {
		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		auto cbRes = this->createBufferInternal(
			&this->scratchBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateFlagsInfo,bufferSize);
		if (!cbRes) return false;
		this->scratchDevice.deviceAddress = this->getBufferDeviceAddress(&this->scratchBuffer);
		return true;
	}
	VkDeviceAddress AnthemAccelerationStruct::getDeviceAddress() const {
		return this->asDeviceAddr.deviceAddress;
	}
}