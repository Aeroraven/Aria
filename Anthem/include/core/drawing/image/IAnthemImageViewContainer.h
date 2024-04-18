#pragma once
#include "../../base/AnthemBaseImports.h"

namespace Anthem::Core {
	class IAnthemImageViewContainer {
	public:
		virtual const VkImageView* getImageView() const = 0;
		virtual const VkImageView* getImageViewFrameBuffer() const = 0;
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual uint32_t getLayers() const = 0;
	};
}