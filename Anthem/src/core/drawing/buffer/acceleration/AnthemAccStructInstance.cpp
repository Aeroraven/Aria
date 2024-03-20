#include "../../../../../include/core/drawing/buffer/acceleration/AnthemAccStructInstance.h"
namespace Anthem::Core {
	bool AnthemAccStructInstance::destroyBuffer() {
		return this->destroyBufferInternal(&this->instanceBuffer);
	}
	bool AnthemAccStructInstance::createInstanceInfoBuffers(std::vector<AnthemBottomLevelAccStruct*> bottomAs, std::vector<std::vector<float>> transform){
		std::vector<VkAccelerationStructureInstanceKHR> lst;
		for (auto i : AT_RANGE2(bottomAs.size())) {
			VkAccelerationStructureInstanceKHR asInstance;
			VkTransformMatrixKHR data;
			memcpy(data.matrix, transform[i].data(), sizeof(float) * 12);
			asInstance.accelerationStructureReference = bottomAs[i]->getDeviceAddress();
			asInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			asInstance.instanceCustomIndex = i;
			asInstance.instanceShaderBindingTableRecordOffset = 0;
			asInstance.mask = 0xFF;
			asInstance.transform = data;

			lst.push_back(asInstance);
		}
		primCnts = bottomAs.size();
		
		VkMemoryAllocateFlagsInfo allocFlags{};
		allocFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		allocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		this->createBufferInternal(
			&this->instanceBuffer,
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocFlags,
			sizeof(VkAccelerationStructureInstanceKHR)*bottomAs.size()
		);
		this->copyDataToBufferInternal(&this->instanceBuffer, lst.data(), sizeof(VkAccelerationStructureInstanceKHR) * bottomAs.size(), true);
		this->instanceBda.deviceAddress = this->getBufferDeviceAddress(&this->instanceBuffer);

		asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		asGeometry.pNext = nullptr;
		asGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		asGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		asGeometry.geometry.instances.data = this->instanceBda;
		asGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
		return true;
	}

	VkAccelerationStructureGeometryKHR AnthemAccStructInstance::getGeometry() const {
		return this->asGeometry;
	}
	uint32_t AnthemAccStructInstance::getPrimitiveCounts() const {
		return primCnts;
	}
}