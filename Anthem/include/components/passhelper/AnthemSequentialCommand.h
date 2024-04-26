#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/renderer/AnthemSimpleToyRenderer.h"
#include "../../core/utils/AnthemUtlTimeOps.h"
#include "../camera/AnthemCamera.h"
#include "AnthemExecutionType.h"

namespace Anthem::Components::PassHelper {
	using namespace Anthem::Core;

	struct AnthemSequentialCommandEntry {
		uint32_t commandBufferIndex;
		AnthemSequentialCommandEntryType type;
	};

	class AnthemSequentialCommand {
	private:
		AnthemSimpleToyRenderer* rd;
		std::vector<AnthemSequentialCommandEntry> seq;
		std::vector<AnthemSemaphore*> semaphores;
		AnthemFence* fence;
		bool graphicsOnly = false;

	public:
		void markGraphicsOnly();
		AnthemSequentialCommand(AnthemSimpleToyRenderer* renderer);
		void setSequence(const std::vector<AnthemSequentialCommandEntry>& seq);
		void executeCommandToStage(uint32_t frameImageIdx, bool forceFence,bool useImGui,AnthemSwapchainFramebuffer* swapchainFb);
		void waitExecutionToStage();
	};
}