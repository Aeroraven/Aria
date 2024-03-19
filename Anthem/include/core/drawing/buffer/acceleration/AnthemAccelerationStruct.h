#pragma once
#include "../../../base/AnthemBaseImports.h"
#include "../../../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../../../utils/AnthemUtlCommandBufferReqBase.h"
#include "../AnthemGeneralBufferBase.h"

namespace Anthem::Core {
	class AnthemAccelerationStruct:
	public virtual AnthemGeneralBufferBase,
	public virtual Util::AnthemUtlCommandBufferReqBase{
	protected:
		AnthemGeneralBufferProp asBuffer;
		VkAccelerationStructureKHR asHandle;
		VkDeviceOrHostAddressKHR asDeviceAddr;

		AnthemGeneralBufferProp scratchBuffer;
		VkDeviceOrHostAddressKHR scratchDevice;

	public:
		bool createAccelerationStructBuffer(uint32_t bufferSize);
		bool createScratchBuffer(uint32_t bufferSize);
		VkDeviceAddress getDeviceAddress() const;
		VkAccelerationStructureKHR getHandle() const { return this->asHandle; }
		uint32_t virtual calculateBufferSize() { return 0; }
		bool destroyBuffer();
	};
}