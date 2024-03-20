#pragma once
#include "../../../../include/core/pipeline/raytracing/AnthemRayTracingShaders.h"

namespace Anthem::Core {
	bool AnthemRayTracingShaders::destroyShader(AnthemLogicalDevice* device) {
		for (auto p : this->shaderModules) {
			vkDestroyShaderModule(device->getLogicalDevice(), p, nullptr);
		}
		return true;
	}
	bool AnthemRayTracingShaders::loadShaderGroup(AnthemLogicalDevice* device, AnthemRayTracingShaderGroupType groupType,
		const std::vector<std::pair<std::string, AnthemRayTracingShaderType>>& loadCfg) {
		std::vector<uint32_t> shaderId;
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;

		if (groupType == AT_RTSG_RAYGEN) {
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroupSize.raygenSize++;
		}
		if (groupType == AT_RTSG_MISS) {
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroupSize.missSize++;
		}
		if (groupType == AT_RTSG_HIT) {
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			shaderGroupSize.hitSize++;
		}
		if (groupType == AT_RTSG_CALLABLE) {
			shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			shaderGroupSize.missSize++;
		}

		for (auto p : loadCfg) {
			std::vector<char> data;
			this->readFile(p.first, &data);
			VkPipelineShaderStageCreateInfo shaderStage{};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.pName = "main";
			std::optional<VkShaderModule> shader = std::make_optional<VkShaderModule>();
			this->createSingleShaderModule(device, &data, &shader);
			shaderStage.module = shader.value();
			shaderModules.push_back(shaderStage.module);
			uint32_t shaderId = static_cast<uint32_t>(shaderModules.size()) - 1;

			if (p.second == AT_RTST_RAYGEN) {
				shaderStage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
				shaderGroup.generalShader = shaderId;
			}
			if (p.second == AT_RTST_MISS) {
				shaderStage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
				shaderGroup.generalShader = shaderId;
			}
			if (p.second == AT_RTST_CLOSEHIT) {
				shaderStage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
				shaderGroup.closestHitShader = shaderId;
			}
			if (p.second == AT_RTST_CALLABLE) {
				shaderStage.stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
				shaderGroup.generalShader = shaderId;
			}
			if (p.second == AT_RTST_INTERSECTION) {
				shaderStage.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
				shaderGroup.intersectionShader = shaderId;
			}
			if (p.second == AT_RTST_ANYHIT) {
				shaderStage.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
				shaderGroup.anyHitShader = shaderId;
			}
			shaderStages.push_back(shaderStage);
		}
		shaderGroups.push_back(shaderGroup);
		return true;
	}
	std::vector<VkPipelineShaderStageCreateInfo> AnthemRayTracingShaders::getShaderStages() const {
		return this->shaderStages;
	}
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> AnthemRayTracingShaders::getShaderGroups() const {
		return this->shaderGroups;
	}
	const AnthemRayTracingShaderGroupSizeInfo AnthemRayTracingShaders::getShaderGroupSizes() const {
		return this->shaderGroupSize;
	}

}