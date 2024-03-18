#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlFileReaderBase.h"
namespace Anthem::Core {
	enum AnthemRayTracingShaderType {
		AT_RTST_RAYGEN = 1,
		AT_RTST_MISS = 2,
		AT_RTST_CLOSEHIT = 3
	};

	class AnthemRayTracingShaders : private virtual Util::AnthemUtlFileReaderBase {
	private:
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
	public:

	};
}