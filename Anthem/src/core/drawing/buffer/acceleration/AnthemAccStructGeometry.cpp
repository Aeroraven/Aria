#include "../../../../../include/core/drawing/buffer/acceleration/AnthemAccStructGeometry.h"
namespace Anthem::Core {

	bool AnthemAccStructGeometry::createGeometryInfoBuffers(int vertexStride, std::vector<float> vertices, std::vector<uint32_t>indices, std::vector<float>transform) {
		void* dataVectors[3] = { vertices.data(),indices.data(),transform.data()};
		AnthemGeneralBufferProp* destBuffer[3] = { &vertexBuffer,&indexBuffer,&transformBuffer };
		size_t bufferSizes[3] = { vertices.size() * sizeof(float),indices.size() * sizeof(uint32_t),transform.size() * sizeof(float) };
		VkDeviceOrHostAddressConstKHR* deviceAddrRecv[3] = { &vertexBda,&indexBda,&transformBda };

		for (auto i : AT_RANGE(3)) {
			auto cbRes = this->createBufferInternal(
				destBuffer[i],
				VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				nullptr, bufferSizes[i]
			);
			auto cpRes = this->copyDataToBuffer(destBuffer[i], dataVectors[i], bufferSizes[i], true);
			deviceAddrRecv[i]->deviceAddress = this->getBufferDeviceAddress(destBuffer[i]);
		}
		this->vertexStride = vertexStride;
		this->maxVertex = vertices.size() - 1;
		return true;
	}
	bool AnthemAccStructGeometry::createTriangularGeometry() {
		VkAccelerationStructureGeometryKHR asGeometry = {};
		asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		asGeometry.flags = 0;
		asGeometry.pNext = nullptr;
		asGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		asGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		asGeometry.geometry.triangles.pNext = nullptr;
		asGeometry.geometry.triangles.maxVertex = this->maxVertex;
		asGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		asGeometry.geometry.triangles.indexData = this->indexBda;
	}
}