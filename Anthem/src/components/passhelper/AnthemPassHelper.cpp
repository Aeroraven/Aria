#include "../../../include/components/passhelper/AnthemPassHelper.h"
namespace Anthem::Components::PassHelper {
	AnthemPassHelper::AnthemPassHelper(AnthemSimpleToyRenderer* renderer, uint32_t copies) {
		this->rd = renderer;
		this->copies = copies;
		
		ANTH_TODO("Memory Leaks");
		this->descDepth = new AnthemDescriptorPool * [copies];
		this->depth = new AnthemDepthBuffer * [copies];
		renderTargets.reserve(copies);
		renderTargets.resize(copies);
		descLayout.reserve(copies);
		descLayout.resize(copies);
		frameBuffers.reserve(copies);
		frameBuffers.resize(copies);
		cmdIdx = new uint32_t[copies];
	}
	void AnthemPassHelper::setRenderTargets(const std::vector<const IAnthemImageViewContainer*>& targets, int destCopy) {
		if (destCopy == -1) {
			for (auto& p : renderTargets) {
				p = targets;
			}
		}
		else {
			renderTargets[destCopy] = targets;
		}
	}
	void AnthemPassHelper::setDescriptorLayouts(const std::vector<AnthemDescriptorSetEntry>& layouts, int destCopy) {
		if (destCopy == -1) {
			for (auto& p : descLayout) {
				p = layouts;
			}
		}
		else {
			descLayout[destCopy] = layouts;
		}
	}
	void AnthemPassHelper::buildGraphicsPipeline() {
		for (auto i : AT_RANGE2(2)) {
			rd->drAllocateCommandBuffer(&cmdIdx[i]);
		}
		rd->createShader(&shader, &shaderPath);
		if (this->enableDepthSampler) {
			for (auto i : AT_RANGE2(copies)) {
				rd->createDescriptorPool(&descDepth[i]);
				rd->createDepthBufferWithSampler(&depth[i], descDepth[i], 0, false);
			}
		}
		else {
			for (auto i : AT_RANGE2(copies)) {
				rd->createDepthBuffer(&depth[i], false);
			}
		}
		rd->setupRenderPass(&pass, &passOpt, depth[0]);
		
		if (passOpt.renderPassUsage == AT_ARPAA_FINAL_PASS) {
			rd->createSwapchainImageFramebuffers(&swapchainFb, pass, depth[0]);
		}
		else {
			for (auto i : AT_RANGE2(copies)) {
				rd->createSimpleFramebufferA(&frameBuffers[i], renderTargets[i], pass, depth[i]);
			}
		}
		rd->createGraphicsPipelineCustomized(&pipeline, descLayout[0], {}, pass, shader, vxLayout, &pipeOpt);
	}
	uint32_t AnthemPassHelper::getCommandIndex(uint32_t copy) {
		return cmdIdx[copy];
	}
	AnthemDescriptorPool* AnthemPassHelper::getDepthDescriptor(uint32_t id) {
		return descDepth[id];
	}
	void AnthemPassHelper::recordCommands(std::function<void(uint32_t)> injectedCommands) {
		for (auto i : AT_RANGE2(copies)) {
			auto ci = cmdIdx[i];
			rd->drStartCommandRecording(ci);
			if (passOpt.renderPassUsage == AT_ARPAA_FINAL_PASS) {
				rd->drStartRenderPass(pass, swapchainFb->getFramebufferObjectUnsafe(i), ci, false);
			}
			else {
				rd->drStartRenderPass(pass,frameBuffers[i], ci, false);
			}
			rd->drBindGraphicsPipeline(pipeline, ci);
			rd->drBindDescriptorSetCustomizedGraphics(descLayout[i], pipeline, ci);
			rd->drSetViewportScissorFromSwapchain(ci);
			injectedCommands(ci);
			rd->drEndRenderPass(ci);
			rd->drEndCommandRecording(ci);
		}
	}
}