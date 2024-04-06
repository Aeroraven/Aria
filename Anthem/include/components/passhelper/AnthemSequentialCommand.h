#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/renderer/AnthemSimpleToyRenderer.h"
#include "../../core/utils/AnthemUtlTimeOps.h"
#include "../camera/AnthemCamera.h"

namespace Anthem::Components::PassHelper {
	using namespace Anthem::Core;
	enum AnthemSequentialCommandEntryType {
		ATC_ASCE_GRAPHICS = 1,
		ATC_ASCE_COMPUTE = 2
	};

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
	public:
		AnthemSequentialCommand(AnthemSimpleToyRenderer* renderer);
		void setSequence(const std::vector<AnthemSequentialCommandEntry>& seq);
		void executeCommandToStage(uint32_t frameImageIdx, bool forceFence);
		void waitExecutionToStage();
	};
}