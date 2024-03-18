#pragma once
#include "../../../base/AnthemBaseImports.h"
#include "../../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../../utils/AnthemUtlCommandBufferReqBase.h"
#include "../AnthemGeneralBufferBase.h"
#include "./AnthemBottomLevelAccStruct.h"

namespace Anthem::Core {
	class AnthemAccStructInstance :
	public virtual AnthemGeneralBufferBase {
	protected:
		AnthemGeneralBufferProp instanceBuffer;
		VkDeviceOrHostAddressConstKHR instanceBda;
		AnthemBottomLevelAccStruct* blas;

		VkAccelerationStructureInstanceKHR asInstance;
		VkAccelerationStructureGeometryKHR asGeometry;
	public:
		bool createInstanceInfoBuffers(AnthemBottomLevelAccStruct* bottomAs, std::vector<float> transform);
		VkAccelerationStructureGeometryKHR getGeometry() const;
		uint32_t getPrimitiveCounts() const;
	};
}