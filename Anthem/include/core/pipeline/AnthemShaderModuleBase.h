#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlFileReaderBase.h"
#include "../base/AnthemLogicalDevice.h"

namespace Anthem::Core {
	class AnthemShaderModuleBase {
	public:
		bool createSingleShaderModule(AnthemLogicalDevice* device, std::vector<char>* shaderCode, std::optional<VkShaderModule>* shaderModule);
	};
}