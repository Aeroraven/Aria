#include "../../../include/components/postprocessing/AnthemBloom.h"
namespace Anthem::Components::Postprocessing {
	void AnthemBloom::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "bloom\\bloom.";
			st += x;
			st += ".hlsl.spv";
			return st;
			};
		shaderPath.fragmentShader = getShader("up.frag");
		shaderPath.vertexShader = getShader("up.vert");
		rd->createShader(&shader, &shaderPath);
	}
	AnthemBloom::AnthemBloom(AnthemSimpleToyRenderer* p, uint32_t cmdCopies, uint32_t layers,uint32_t width,uint32_t height) :
		AnthemPostprocessPass(p, cmdCopies) {
		this->layers = layers;
		this->width = width;
		this->height = height;
	}
	void AnthemBloom::setSrcImage(AnthemDescriptorPool* src) {
		this->srcImg = src;

	}
	void AnthemBloom::prepareDownsample() {
		ANTH_TODO("Create geometry twice");
		prepareGeometry();
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "bloom\\bloom.";
			st += x;
			st += ".hlsl.spv";
			return st;
			};
		shaderDownPath.fragmentShader = getShader("down.frag");
		shaderDownPath.vertexShader = getShader("down.vert");
		rd->createShader(&shaderDown, &shaderDownPath);

		downFramebuffers.reserve(layers);
		downFramebuffers.resize(layers);
		downRenderPasses.reserve(layers);
		downRenderPasses.resize(layers);
		downPipelines.reserve(layers);
		downPipelines.resize(layers);
		downColorAttachments.reserve(layers);
		downColorAttachments.resize(layers);
		downDescriptorPools.reserve(layers);
		downDescriptorPools.resize(layers);
		downViewports.reserve(layers);
		downViewports.resize(layers);
		downPc.reserve(layers);
		downPc.resize(layers);
		downPopt.enableDepthTesting = false;

		rd->createDescriptorPool(&downChain);
		std::vector<AnthemImageContainer*> imgc;
		int bw = width/2, bh = height/2;
		for (auto i : AT_RANGE2(layers)) {
			rd->createDescriptorPool(&downDescriptorPools[i]);
			rd->createColorAttachmentImage(&downColorAttachments[i], downDescriptorPools[i], 0, AT_IF_SIGNED_FLOAT32, false, -1, false, bw, bh);
			rd->createViewportCustom(&downViewports[i], bw, bh, 0, 1);
		}
		for (auto i : AT_RANGE2(layers)) {
			downRopt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
			downRopt.clearDepthAttachmentOnLoad = false;
			downRopt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
			rd->setupRenderPass(&downRenderPasses[i], &downRopt, nullptr);
			rd->createSimpleFramebufferA(&downFramebuffers[i], { downColorAttachments[i] }, downRenderPasses[i], nullptr);
			std::vector<AnthemDescriptorSetEntry> dseEntries{};
			if (i == 0) {
				dseEntries.push_back({ srcImg,AT_ACDS_SAMPLER,0 });
			}
			else {
				dseEntries.push_back({ downDescriptorPools[i - 1],AT_ACDS_SAMPLER,0 });
			}
			rd->createPushConstant(&downPc[i]);
			float data = i;
			downPc[i]->setConstant(&data);
			downPc[i]->enableShaderStage(AT_APCS_VERTEX);
			downPc[i]->enableShaderStage(AT_APCS_FRAGMENT);
			rd->createGraphicsPipelineCustomized(&downPipelines[i], dseEntries,{downPc[i]}, downRenderPasses[i], shaderDown, this->vx, &downPopt);
			imgc.push_back(downColorAttachments[i]);
		}
		rd->addSamplerArrayToDescriptor(imgc, downChain, 0, -1);
		
	}
	void AnthemBloom::prepare(bool offscreen) {
		prepareDownsample();
		std::vector<AnthemDescriptorSetEntry> dseEntries{};
		dseEntries.push_back({ srcImg,AT_ACDS_SAMPLER,0 });
		dseEntries.push_back({ downChain,AT_ACDS_SAMPLER,0 });
		addInput(dseEntries);
		AnthemPostprocessPass::prepare(offscreen);
	}
	void AnthemBloom::recordCommand() {
		for (auto k = 0; auto i : cmdIdx) {
			rd->drStartCommandRecording(i);
			for (auto j : AT_RANGE2(layers)) {
				std::vector<AnthemDescriptorSetEntry> dseEntries{};
				if (j == 0) {
					dseEntries.push_back({ srcImg,AT_ACDS_SAMPLER,0 });
				}
				else {
					dseEntries.push_back({ downDescriptorPools[j - 1],AT_ACDS_SAMPLER,0 });
				}
				rd->drPushConstants(downPc[j], downPipelines[j], i);
				rd->drStartRenderPass(downRenderPasses[j], downFramebuffers[j], i, false);
				rd->drBindGraphicsPipeline(downPipelines[j], i);
				rd->drSetViewportScissor(downViewports[j], i);
				rd->drBindDescriptorSetCustomizedGraphics(dseEntries, downPipelines[j], i);
				rd->drBindVertexBuffer(vx, i);
				rd->drBindIndexBuffer(ix, i);
				rd->drDraw(ix->getIndexCount(), i);
				rd->drEndRenderPass(i);
			}
			rd->drStartRenderPass(pass, fbSwapchain->getFramebufferObjectUnsafe(k), i, this->enabledMsaa);
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
}