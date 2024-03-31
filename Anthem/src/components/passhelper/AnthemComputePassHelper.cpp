#include "../../../include/components/passhelper/AnthemComputePassHelper.h"
namespace Anthem::Components::PassHelper {
	AnthemComputePassHelper::AnthemComputePassHelper(AnthemSimpleToyRenderer* renderer, uint32_t copies) {
		ANTH_TODO("Memory Leaks");
		this->rd = renderer;
		this->copies = copies;
		descLayout.reserve(copies);
		descLayout.resize(copies);
		cmdIdx = new uint32_t[copies];
	}
	void AnthemComputePassHelper::setDescriptorLayouts(const std::vector<AnthemDescriptorSetEntry>& layouts, int destCopy) {
		if (destCopy == -1) {
			for (auto& p : descLayout) {
				p = layouts;
			}
		}
		else {
			descLayout[destCopy] = layouts;
		}
	}
	uint32_t AnthemComputePassHelper::getCommandIndex(uint32_t id) const{
		return cmdIdx[id];
	}
	void AnthemComputePassHelper::buildComputePipeline() {
		for (auto i : AT_RANGE2(2)) {
			rd->drAllocateCommandBuffer(&cmdIdx[i]);
		}
		rd->createShader(&shader, &shaderPath);
		rd->createComputePipelineCustomized(&pipeline, descLayout[0], shader);
	}
	void AnthemComputePassHelper::recordCommand() {
		for (auto i : AT_RANGE2(copies)) {
			auto ci = cmdIdx[i];
			rd->drStartCommandRecording(ci);
			rd->drBindComputePipeline(pipeline, ci);
			rd->drBindDescriptorSetCustomizedCompute(descLayout[i], pipeline, ci);
			rd->drComputeDispatch(ci, workGroupSize[0], workGroupSize[1], workGroupSize[2]);
			rd->drEndCommandRecording(ci);
		}
	}
}