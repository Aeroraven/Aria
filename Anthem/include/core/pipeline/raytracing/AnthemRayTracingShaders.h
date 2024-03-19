#pragma once
#include "../../base/AnthemBaseImports.h"
#include "../../utils/AnthemUtlFileReaderBase.h"
#include "../../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../AnthemShaderModuleBase.h"
namespace Anthem::Core {
	enum AnthemRayTracingShaderType {
		AT_RTST_RAYGEN = 1,
		AT_RTST_MISS = 2,
		AT_RTST_CLOSEHIT = 3
	};

	class AnthemRayTracingShaders : 
	private virtual Util::AnthemUtlFileReaderBase,
	private virtual AnthemShaderModuleBase {

	private:
		std::vector<VkShaderModule> shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};

	public:

		void loadShader(AnthemLogicalDevice* device,std::string path, AnthemRayTracingShaderType type);
		std::vector<VkPipelineShaderStageCreateInfo> getShaderStages() const;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> getShaderGroups() const;
		bool destroyShader(AnthemLogicalDevice* device);
	};
}