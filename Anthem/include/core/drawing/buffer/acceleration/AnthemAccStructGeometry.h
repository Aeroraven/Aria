#pragma once
#include "../../../base/AnthemBaseImports.h"
#include "../../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../../utils/AnthemUtlCommandBufferReqBase.h"
#include "../AnthemGeneralBufferBase.h"

namespace Anthem::Core {
	class AnthemAccStructGeometry :
	public virtual AnthemGeneralBufferBase {
	protected:
		AnthemGeneralBufferProp vertexBuffer;
		AnthemGeneralBufferProp indexBuffer;
		AnthemGeneralBufferProp transformBuffer;

		VkDeviceOrHostAddressConstKHR vertexBda;
		VkDeviceOrHostAddressConstKHR indexBda;
		VkDeviceOrHostAddressConstKHR transformBda;

		uint32_t vertexStride = 0;
		uint32_t maxVertex = 0;
	public:
		bool createGeometryInfoBuffers(int vertexStride, std::vector<float> vertices, std::vector<uint32_t>indices, std::vector<float>transform);
		bool createTriangularGeometry();
	};
}