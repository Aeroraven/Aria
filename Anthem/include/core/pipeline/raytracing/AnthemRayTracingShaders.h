#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlFileReaderBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../AnthemShaderModuleBase.h"
namespace Anthem::Core {
	enum AnthemRayTracingShaderType {
		AT_RTST_RAYGEN = 1,
		AT_RTST_MISS = 2,
		AT_RTST_CLOSEHIT = 3,
		AT_RTST_INTERSECTION = 4,
		AT_RTST_CALLABLE = 5,
		AT_RTST_ANYHIT = 6
	};

	enum AnthemRayTracingShaderGroupType {
		AT_RTSG_RAYGEN = 1,
		AT_RTSG_MISS = 2,
		AT_RTSG_HIT = 3,
		AT_RTSG_CALLABLE = 4
	};

	struct AnthemRayTracingShaderGroupSizeInfo {
		uint32_t raygenSize = 0;
		uint32_t missSize = 0;
		uint32_t hitSize = 0;
		uint32_t callableSize = 0;
	};

	class AnthemRayTracingShaders : 
	private virtual Util::AnthemUtlFileReaderBase,
	private virtual AnthemShaderModuleBase {

	private:
		std::vector<VkShaderModule> shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
		AnthemRayTracingShaderGroupSizeInfo shaderGroupSize{};

	public:

		bool loadShaderGroup(AnthemLogicalDevice* device, AnthemRayTracingShaderGroupType groupType,
			const std::vector<std::pair<std::string, AnthemRayTracingShaderType>>& loadCfg);
		std::vector<VkPipelineShaderStageCreateInfo> getShaderStages() const;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> getShaderGroups() const;
		const AnthemRayTracingShaderGroupSizeInfo getShaderGroupSizes() const;
		bool destroyShader(AnthemLogicalDevice* device);
	};
}