#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/renderer/AnthemSimpleToyRenderer.h"
#include "../../core/utils/AnthemUtlTimeOps.h"
#include "../camera/AnthemCamera.h"
#include "AnthemExecutionType.h"

namespace Anthem::Components::PassHelper {
	using namespace Anthem::Core;

	struct AnthemDualExecSeq {
		std::vector<uint32_t> cmdBufs;
		AnthemSequentialCommandEntryType execType;
		int dependency = -1;
	};

	class AnthemDualSeqCommand {
	private:
		AnthemFence* fence = nullptr;
		std::vector<AnthemSemaphore*> syncs;
		std::vector<AnthemDualExecSeq> seq;
		AnthemSimpleToyRenderer* rd;

		std::vector<int> syncToWait;
		std::vector<int> syncToSignal;

	public:
		AnthemDualSeqCommand(AnthemSimpleToyRenderer* renderer);
		void setSequence(const std::vector<AnthemDualExecSeq>& seq);
		void executeCommandToStage(uint32_t frameImageIdx, bool forceFence, bool useImGui, AnthemSwapchainFramebuffer* swapchainFb);
	};
}