#include "../../../include/components/postprocessing/AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
	AnthemPostprocessPass::AnthemPostprocessPass(AnthemSimpleToyRenderer * p, uint32_t cmdCopies) {
		this->rd = p;
		this->cmdCopies = cmdCopies;
		this->cmdIdx.reserve(cmdCopies);
		this->cmdIdx.resize(cmdCopies);
		this->inputs.reserve(cmdCopies);
		this->inputs.resize(cmdCopies);
	}
	void AnthemPostprocessPass::prepareGeometry() {
		rd->createVertexBuffer(&vx);
		rd->createIndexBuffer(&ix);
		vx->setTotalVertices(4);
		vx->insertData(0, { -1,-1,0,1 });
		vx->insertData(1, { 1,-1,0,1 });
		vx->insertData(2, { 1,1,0,1 });
		vx->insertData(3, { -1,1,0,1 });
		ix->setIndices({ 0,1,2,2,3,0 });
	}

	void AnthemPostprocessPass::addInput(std::vector<AnthemDescriptorSetEntry> ins,int target) {
		if (target == -1) {
			for (auto& inx : inputs) {
				inx.insert(inx.end(), ins.begin(), ins.end());
			}
		}
		else {
			inputs[target].insert(inputs[target].end(), ins.begin(), ins.end());
		}
	}
	void AnthemPostprocessPass::prepare() {
		for (auto i : AT_RANGE2(cmdCopies)) {
			rd->drAllocateCommandBuffer(&cmdIdx[i]);
		}
		rd->createDepthBuffer(&depthStencil,false);
		prepareGeometry();
		prepareShader();
		prepareRenderPass();
		rd->createSwapchainImageFramebuffers(&fbSwapchain, pass, depthStencil);
		rd->createGraphicsPipelineCustomized(&pipeline, inputs[0], {}, pass, shader, vx, &copt);
	}

	void AnthemPostprocessPass::prepareRenderPass() {
		rd->setupRenderPass(&pass, &ropt, depthStencil);
	}

	void AnthemPostprocessPass::recordCommand() {
		for (auto k = 0; auto i : cmdIdx) {
			rd->drStartCommandRecording(i);
			rd->drStartRenderPass(pass, fbSwapchain->getFramebufferObjectUnsafe(k), i, false);
			rd->drBindGraphicsPipeline(pipeline, i);
			rd->drSetViewportScissorFromSwapchain(i);
			rd->drBindDescriptorSetCustomizedGraphics(inputs[k], pipeline, i);
			rd->drBindVertexBuffer(vx, i);
			rd->drBindIndexBuffer(ix, i);
			rd->drDraw(ix->getIndexCount(), i);
			rd->drEndRenderPass(i);
			rd->drEndCommandRecording(i);
			k++;
		}
	}
	uint32_t AnthemPostprocessPass::getCommandIdx(uint32_t id) const {
		return this->cmdIdx[id];
	}
}