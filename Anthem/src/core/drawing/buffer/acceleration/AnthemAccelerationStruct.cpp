#include "../../../../../include/core/drawing/buffer/acceleration/AnthemAccelerationStruct.h"

namespace Anthem::Core {
	bool AnthemAccelerationStruct::createAccelerationStruct(uint32_t bufferSize) {
		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		return this->createBufferInternal(
			&this->asBuffer, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateFlagsInfo);
	}
}