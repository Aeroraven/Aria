#include "../../../include/components/passhelper/AnthemSequentialCommand.h"
namespace Anthem::Components::PassHelper {
	AnthemSequentialCommand::AnthemSequentialCommand(AnthemSimpleToyRenderer* renderer) {
		this->rd = renderer;
	}
	void AnthemSequentialCommand::setSequence(const std::vector<AnthemSequentialCommandEntry>& seq) {
		this->seq = seq;
		semaphores.reserve(seq.size());
		semaphores.resize(seq.size());
		for (auto i : AT_RANGE2(seq.size())) {
			rd->createSemaphore(&semaphores[i]);
		}
	}
	void AnthemSequentialCommand::executeCommandToStage(uint32_t frameImageIdx, bool forceFence, bool useImGui, AnthemSwapchainFramebuffer* swapchainFb) {
		if (!graphicsOnly) {
			for (auto i : AT_RANGE2(seq.size())) {
				std::vector<const AnthemSemaphore*> toWait = (i == 0) ? std::vector<const AnthemSemaphore*>{} : std::vector<const AnthemSemaphore*>{ semaphores[i - 1] };
				std::vector<AtSyncSemaphoreWaitStage> waitStage = (i == 0) ? std::vector<AtSyncSemaphoreWaitStage>{} : std::vector<AtSyncSemaphoreWaitStage>{ AT_SSW_ALL_COMMAND };

				if (i != seq.size() - 1 || useImGui) {
					AnthemFence* sigFence = ((i == seq.size() - 2 || (i == seq.size() - 1 && useImGui)) && forceFence) ? this->fence : nullptr;
					if (seq[i].type == ATC_ASCE_GRAPHICS) {
						rd->drSubmitCommandBufferGraphicsQueueGeneral2A(seq[i].commandBufferIndex, -1, toWait, waitStage, sigFence, { semaphores[i] });
					}
					else if (seq[i].type == ATC_ASCE_COMPUTE) {
						rd->drSubmitCommandBufferCompQueueGeneralA(seq[i].commandBufferIndex, toWait, { semaphores[i] }, sigFence);
					}
				}
				else {
					if (seq[i].type == ATC_ASCE_GRAPHICS) {
						rd->drSubmitCommandBufferGraphicsQueueGeneralA(seq[i].commandBufferIndex, frameImageIdx, toWait, waitStage);
					}
					else {
						ANTH_LOGE("Baked compute command buffer is not supported now.");
					}
				}
			}
		}
		else {
			std::vector<uint32_t> cmds;
			for (auto i : AT_RANGE2(seq.size()-((useImGui)?0:1))) {
				cmds.push_back(seq[i].commandBufferIndex);
			}
			rd->drBatchedSubmitCommandBuferGraphicsQueueOffscreen(cmds, {}, {}, { semaphores[seq.size() - 1] }, (forceFence) ? this->fence : nullptr);
			if (!useImGui) {
				rd->drSubmitCommandBufferGraphicsQueueGeneralA(seq.back().commandBufferIndex, frameImageIdx, { semaphores[seq.size() - 1] }, { AT_SSW_ALL_COMMAND });
			}
		}
		
		if (useImGui) {
			ImGui::Render();
			ImDrawData* drawData = ImGui::GetDrawData();
			rd->exRenderImGui(frameImageIdx, swapchainFb, { 0,0,0,1 }, drawData);

			uint32_t imguiCmdBuf;
			rd->exGetImGuiCommandBufferIndex(frameImageIdx, &imguiCmdBuf);
			AnthemSemaphore* imguiSemaphore;
			rd->exGetImGuiDrawProgressSemaphore(frameImageIdx, &imguiSemaphore);
			rd->drSubmitCommandBufferGraphicsQueueGeneralA(imguiCmdBuf, frameImageIdx, { semaphores[seq.size() - 1] }, { AT_SSW_ALL_COMMAND });
		}
	}
	void AnthemSequentialCommand::waitExecutionToStage() {
		if (seq.size() >= 2) {
			this->fence->waitAndReset();
		}
	}
	void AnthemSequentialCommand::markGraphicsOnly() {
		this->graphicsOnly = true;
	}
}