#pragma once
#include "../../../base/AnthemBaseImports.h"
#include "../../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../../utils/AnthemUtlCommandBufferReqBase.h"
#include "../AnthemGeneralBufferBase.h"

namespace Anthem::Core {
	class AnthemAccelerationStruct:
	public virtual AnthemGeneralBufferBase{
	protected:
		AnthemGeneralBufferProp asBuffer;
		VkAccelerationStructureKHR asHandle;

	public:
		bool createAccelerationStruct(uint32_t bufferSize);
	};
}