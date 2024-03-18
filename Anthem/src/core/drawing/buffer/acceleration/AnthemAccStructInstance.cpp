#include "../../../../../include/core/drawing/buffer/acceleration/AnthemAccStructInstance.h"
namespace Anthem::Core {
	bool AnthemAccStructInstance::createInstanceInfoBuffers(AnthemBottomLevelAccStruct* bottomAs, std::vector<float> transform) {
		VkTransformMatrixKHR data;
		memcpy(data.matrix, transform.data(), sizeof(float) * 12);
		asInstance.accelerationStructureReference = bottomAs->getDeviceAddress();
		asInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		asInstance.instanceCustomIndex = 0;
		asInstance.instanceShaderBindingTableRecordOffset = 0;
		asInstance.mask = 0xFF;
		asInstance.transform = data;

		this->createBufferInternal(
			&this->instanceBuffer,
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr,
			sizeof(decltype(asInstance))
		);
		this->copyDataToBufferInternal(&this->instanceBuffer, &asInstance, sizeof(decltype(asInstance)), true);
		this->instanceBda.deviceAddress = this->getBufferDeviceAddress(&this->instanceBuffer);

		asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		asGeometry.pNext = nullptr;
		asGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		asGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		asGeometry.geometry.instances.data = this->instanceBda;
		asGeometry.geometry.instances.arrayOfPointers = VK_FALSE;

	}

	VkAccelerationStructureGeometryKHR AnthemAccStructInstance::getGeometry() const {
		return this->asGeometry;
	}
	uint32_t AnthemAccStructInstance::getPrimitiveCounts() const {
		return 1;
	}
}