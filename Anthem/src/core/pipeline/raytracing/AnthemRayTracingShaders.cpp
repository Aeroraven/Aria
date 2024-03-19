#pragma once
#include "../../../../include/core/pipeline/raytracing/AnthemRayTracingShaders.h"

namespace Anthem::Core {
	bool AnthemRayTracingShaders::destroyShader(AnthemLogicalDevice* device) {
		for (auto p : this->shaderModules) {
			vkDestroyShaderModule(device->getLogicalDevice(), p, nullptr);
		}
		return true;
	}
	void AnthemRayTracingShaders::loadShader(AnthemLogicalDevice* device,std::string path, AnthemRayTracingShaderType type) {
		std::vector<char> data;
		this->readFile(path, &data);
		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.pName = "main";
		std::optional<VkShaderModule> shader = std::make_optional<VkShaderModule>();
		this->createSingleShaderModule(device, &data, &shader);
		shaderStage.module = shader.value();
		shaderModules.push_back(shaderStage.module);
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		switch (type) {
		case AT_RTST_RAYGEN:
			shaderStage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size());
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			break;
		case AT_RTST_MISS:
			shaderStage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size());
			shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			break;
		case AT_RTST_CLOSEHIT:
			shaderStage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size());
			shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
			shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
			break;
		default:
			ANTH_LOGE("Not supported");
		}
		shaderStages.push_back(shaderStage);
		shaderGroups.push_back(shaderGroup);
	}
	std::vector<VkPipelineShaderStageCreateInfo> AnthemRayTracingShaders::getShaderStages() const {
		return this->shaderStages;
	}
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> AnthemRayTracingShaders::getShaderGroups() const {
		return this->shaderGroups;
	}

}