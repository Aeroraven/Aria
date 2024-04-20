#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/renderer/AnthemSimpleToyRenderer.h"
#include "../../core/utils/AnthemUtlTimeOps.h"
#include "../camera/AnthemCamera.h"

namespace Anthem::Components::PassHelper {
	using namespace Anthem::Core;

	class AnthemComputePassHelper {
	protected:
		AnthemSimpleToyRenderer* rd;
		AnthemShaderModule* shader;
		std::vector<std::vector<AnthemDescriptorSetEntry>> descLayout;
		uint32_t* cmdIdx;
		uint32_t copies;

	public:
		AnthemComputePipeline* pipeline;
		std::vector<AnthemPushConstant*> pushConstants = {};
		std::array<uint32_t, 3> workGroupSize;
		AnthemShaderFilePaths shaderPath;

	public:
		AnthemComputePassHelper(AnthemSimpleToyRenderer* renderer, uint32_t copies);
		void buildComputePipeline();
		void recordCommand();
		uint32_t getCommandIndex(uint32_t id) const;
		void setDescriptorLayouts(const std::vector<AnthemDescriptorSetEntry>& layouts, int destCopy = -1);
	};
}