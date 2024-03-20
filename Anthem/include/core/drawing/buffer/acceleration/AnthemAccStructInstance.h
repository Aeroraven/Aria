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

		uint32_t primCnts = 0;
		VkAccelerationStructureGeometryKHR asGeometry;
	public:
		bool createInstanceInfoBuffers(std::vector<AnthemBottomLevelAccStruct*> bottomAs, std::vector<std::vector<float>> transform);
		VkAccelerationStructureGeometryKHR getGeometry() const;
		uint32_t getPrimitiveCounts() const;
		uint32_t virtual calculateBufferSize() override { return 0; }
		bool destroyBuffer();
	};
}