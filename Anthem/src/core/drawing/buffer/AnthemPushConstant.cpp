#include "../../../../include/core/drawing/buffer/AnthemPushConstant.h"

namespace Anthem::Core {
	bool AnthemPushConstant::enableShaderStage(AnthemPushConstantStageFlags stage) {
		switch (stage) {
		case AT_APCS_VERTEX:
			this->flags |= VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case AT_APCS_FRAGMENT:
			this->flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		default:
			ANTH_LOGE("Unsupported flag:", stage);
		}
	}
	VkPushConstantRange AnthemPushConstant::getRange() {
		VkPushConstantRange range = {};
		range.stageFlags = this->flags;
		range.size = this->getSize();
		range.offset = 0;
	}
}