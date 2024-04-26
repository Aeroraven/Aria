#include "../../../include/components/passhelper/AnthemDualSeqCommand.h"
namespace Anthem::Components::PassHelper {
	AnthemDualSeqCommand::AnthemDualSeqCommand(AnthemSimpleToyRenderer* renderer) {
		this->rd = renderer;
		rd->createFence(&fence);
	}
	void AnthemDualSeqCommand::setSequence(const std::vector<AnthemDualExecSeq>& seq) {
		this->seq = seq;
		for (auto i : AT_RANGE2(seq.size())) {
			ANTH_ASSERT(seq[i].dependency < i, "Invalid synchronization order");
			ANTH_ASSERT(syncToSignal[i] == -1, "Signal already set");
			ANTH_ASSERT(seq[i].execType != seq[seq[i].dependency].execType, "Synchronization order explicitly set");
			syncToSignal[i] = -1;
			syncToWait[i] = -1;
			if (seq[i].dependency != -1) {
				syncs.push_back(nullptr);
				rd->createSemaphore(&syncs[syncs.size() - 1]);
				syncToSignal[seq[i].dependency] = syncs.size() - 1;
				syncToWait[i] = syncs.size() - 1;
			}
		}
	}
	void AnthemDualSeqCommand::executeCommandToStage(uint32_t frameImageIdx, bool forceFence, bool useImGui, AnthemSwapchainFramebuffer* swapchainFb) {
		for (auto i : AT_RANGE2(seq.size())) {
			std::vector<const AnthemSemaphore*> waitSem;
			std::vector<const AnthemSemaphore*> signalSem;
			std::vector<AtSyncSemaphoreWaitStage> waitStage;
			AnthemFence* fc = (i == seq.size() - 1 && forceFence) ? fence : nullptr;

			if (syncToWait[i] != -1) {
				waitSem.push_back(syncs[syncToWait[i]]);
				waitStage.push_back(AT_SSW_ALL_COMMAND);
			}
			if (syncToSignal[i] != -1) {
				signalSem.push_back(syncs[syncToSignal[i]]);
			}
			if (seq[i].execType == ATC_ASCE_GRAPHICS) {
				rd->drBatchedSubmitCommandBufferGraphicsQueueOffscreen(seq[i].cmdBufs, waitSem, waitStage, signalSem, fc);
			}
			else if (seq[i].execType == ATC_ASCE_COMPUTE) {
				rd->drBatchedSubmitCommandBufferCompQueue(seq[i].cmdBufs, waitSem, waitStage, signalSem, fc);
			}
		}
	}
}