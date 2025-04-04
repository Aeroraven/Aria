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
	void AnthemPostprocessPass::prepare(bool offscreen) {
		for (auto i : AT_RANGE2(cmdCopies)) {
			rd->drAllocateCommandBuffer(&cmdIdx[i]);
		}
		rd->createDepthBuffer(&depthStencil,false);
		prepareGeometry();
		prepareShader();
		
		ropt.storeDepthValues = false;
		copt.enableDepthTesting = false;
		copt.enableDepthWriting = false;
		if (offscreen) {
			ropt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
			ropt.colorAttachmentFormats = { caFormat };
			drawOffscreen = true;
		}
		prepareRenderPass();
		if (!offscreen) {
			this->pipeline = new AnthemGraphicsPipeline * [1];
			rd->createSwapchainImageFramebuffers(&fbSwapchain, pass, depthStencil);
			rd->createGraphicsPipelineCustomized(&pipeline[0], inputs[0], {}, pass, shader, vx, &copt);
		}
		else {
			this->descTarget = new AnthemDescriptorPool * [this->cmdCopies];
			this->targetImage = new AnthemImage * [this->cmdCopies];
			this->fbTarget = new AnthemFramebuffer * [this->cmdCopies];
			this->pipeline = new AnthemGraphicsPipeline * [this->cmdCopies];

			for (auto i : AT_RANGE2(this->cmdCopies)) {
				rd->createDescriptorPool(&descTarget[i]);
				rd->createColorAttachmentImage(&targetImage[i], descTarget[i], 0, caFormat, false, -1,false);
				rd->createSimpleFramebufferA(&fbTarget[i], { this->targetImage[i] }, this->pass, nullptr);
				rd->createGraphicsPipelineCustomized(&pipeline[i], inputs[0], {}, pass, shader, vx, &copt);
			}
		}
	}

	void AnthemPostprocessPass::prepareRenderPass() {
		if (ropt.renderPassUsage == AT_ARPAA_FINAL_PASS) {
			rd->setupRenderPass(&pass, &ropt, depthStencil);
		}
		else {
			rd->setupRenderPass(&pass, &ropt, nullptr);
		}
		
	}
	void AnthemPostprocessPass::enableMsaa() {
		ANTH_TODO("Forced type cast");
		enabledMsaa = true;
		rd->createColorAttachmentImage(&msaaColorImage, nullptr, 0, AT_IF_SWAPCHAIN, true, -1);
		ropt.msaaType = AT_ARPMT_MSAA;
		ropt.msaaColorAttachment = msaaColorImage;
		ropt.msaaSamples = (VkSampleCountFlagBits)msaaColorImage->getSampleCount();
	}
	void AnthemPostprocessPass::recordCommand() {
		for (auto k = 0; auto i : cmdIdx) {
			rd->drStartCommandRecording(i);
			rd->drStartRenderPass(pass, fbSwapchain->getFramebufferObjectUnsafe(k), i,this->enabledMsaa);
			rd->drBindGraphicsPipeline(pipeline[0], i);
			rd->drSetViewportScissorFromSwapchain(i);
			rd->drBindDescriptorSetCustomizedGraphics(inputs[k], pipeline[0], i);
			rd->drBindVertexBuffer(vx, i);
			rd->drBindIndexBuffer(ix, i);
			rd->drDraw(ix->getIndexCount(), i);
			rd->drEndRenderPass(i);
			rd->drEndCommandRecording(i);
			k++;
		}
	}

	void AnthemPostprocessPass::recordCommandOffscreen() {
		for (auto k = 0; auto i : cmdIdx) {
			rd->drStartCommandRecording(i);
			rd->drStartRenderPass(pass, fbTarget[k], i, false);
			rd->drBindGraphicsPipeline(pipeline[k], i);
			rd->drSetViewportScissorFromSwapchain(i);
			rd->drBindDescriptorSetCustomizedGraphics(inputs[k], pipeline[k], i);
			rd->drBindVertexBuffer(vx, i);
			rd->drBindIndexBuffer(ix, i);
			rd->drDraw(ix->getIndexCount(), i);
			rd->drEndRenderPass(i);
			//rd->drSetImageLayoutSimple(targetImage[k], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i);
			//targetImage[k]->registerMipmapGenCommand(i);
			rd->drEndCommandRecording(i);
			k++;
		}
	}
	void AnthemPostprocessPass::setAttachmentPrecision(AnthemImageFormat atif) {
		this->caFormat = atif;
	}

	uint32_t AnthemPostprocessPass::getCommandIdx(uint32_t id) const {
		return this->cmdIdx[id];
	}

	AnthemDescriptorPool* AnthemPostprocessPass::getColorAttachmentDescId(uint32_t id) const {
		return this->descTarget[id];
	}
	AnthemSwapchainFramebuffer* AnthemPostprocessPass::getSwapchainFb() const {
		return this->fbSwapchain;
	}
}