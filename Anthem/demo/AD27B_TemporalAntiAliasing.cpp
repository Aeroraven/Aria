#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/components/performance/AnthemFrameRateMeter.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"
#include "../include/core/drawing/buffer/impl/AnthemVertexBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemInstancingVertexBufferImpl.h"
#include "../include/core/drawing/image/AnthemImage.h"
#include "../include/core/drawing/image/AnthemImageCubic.h"
#include "../include/core/drawing/buffer/impl/AnthemShaderStorageBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemPushConstantImpl.h"
#include "../include/components/utility/AnthemSimpleModelIntegrator.h"
#include "../include/components/postprocessing/AnthemTAA.h"
#include "../include/components/math/AnthemLowDiscrepancySequence.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::Components::Postprocessing;
using namespace Anthem::Components::Math;
using namespace Anthem::External;
using namespace Anthem::Core;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "taa\\taademo.";
	st += x;
	st += ".hlsl.spv";
	return st;
}

struct Stage {
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemCamera camLight = AnthemCamera(AT_ACPT_PERSPECTIVE);

	std::vector<AnthemGLTFLoaderParseResult> model;
	AnthemSimpleModelIntegrator ldModel;

	AnthemDescriptorPool* descImage = nullptr;
	AnthemImage** image = nullptr;
	AnthemDepthBuffer* depthMain;

	AnthemShaderFilePaths spMain;
	AnthemShaderModule* sdMain;
	AnthemRenderPassSetupOption roptMain;
	AnthemGraphicsPipelineCreateProps coptMain;
	AnthemRenderPass* passMain;

	AnthemFramebuffer** fbMain;
	AnthemGraphicsPipeline* pipeMain;

	AnthemDescriptorPool* descMain;
	AnthemUniformBufferImpl<
		AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>,
		AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>
	>* uniStage;

	AnthemDescriptorPool** descTgt;
	AnthemImage** target;

	uint32_t* cmdIdx;
	AnthemSemaphore* drawComplete;
	AnthemFence* drawReady;

	// TAA
	std::unique_ptr<AnthemTAA> taa;
	std::vector<std::pair<float, float>> halton;
	uint32_t counter = 0;
}st;

void initialize() {
	st.cfg.demoName = "27-B. Temporal Anti-Aliasing";
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);

	st.camera.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 0.9, -1.5);
	st.camera.specifyFrontEyeRay(0, 0, 1);

	st.rd.createDescriptorPool(&st.descMain);
	st.rd.createUniformBuffer(&st.uniStage, 0, st.descMain, -1);
}

void loadModel() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\kirara\\kirara-2.gltf").c_str());
	loader.parseModel(config, st.model);
	std::vector<AnthemUtlSimpleModelStruct> rp;
	for (auto& p : st.model)rp.push_back(p);
	st.ldModel.loadModel(&st.rd, rp, -1);

	st.rd.createDescriptorPool(&st.descImage);
	st.image = new AnthemImage * [st.model.size()];
	std::vector<AnthemImageContainer*> imgContainer;
	for (int i = 0; i < st.model.size(); i++) {
		std::unique_ptr<AnthemImageLoader> loader = std::make_unique<AnthemImageLoader>();
		uint32_t texWidth, texHeight, texChannels;
		uint8_t* texData;

		std::string texPath = st.model[i].basePath + st.model[i].pbrBaseColorTexPath;
		if (st.model[i].pbrBaseColorTexPath == "") {
			texPath = ANTH_ASSET_DIR;
			texPath += "cat.jpg";
		}
		loader->loadImage(texPath.c_str(), &texWidth, &texHeight, &texChannels, &texData);
		st.rd.createTexture(&st.image[i], st.descImage, texData, texWidth, texHeight, texChannels, 0, false, false, AT_IF_SRGB_UINT8, -1, true);
		imgContainer.push_back(st.image[i]);
	}
	st.rd.addSamplerArrayToDescriptor(imgContainer, st.descImage, 0, -1);
}

void prepareJitterSequence() {
	AnthemHaltonSequence first(2, 8), second(3, 8);
	for (const auto& [a, b] : AT_ZIP(first, second)) {
		st.halton.push_back({ a,b });
	}
}

void createMainStage() {
	st.spMain.fragmentShader = getShader("main.frag");
	st.spMain.vertexShader = getShader("main.vert");
	st.rd.createShader(&st.sdMain, &st.spMain);

	st.roptMain.clearDepthAttachmentOnLoad = true;
	st.roptMain.clearStencilAttachmentOnLoad = true;
	st.roptMain.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.coptMain.enableDynamicStencilTesting = false;

	st.rd.createDepthStencilBuffer(&st.depthMain, false);
	st.rd.setupRenderPass(&st.passMain, &st.roptMain, st.depthMain);

	st.descTgt = new AnthemDescriptorPool * [st.cfg.vkcfgMaxImagesInFlight];
	st.target = new AnthemImage * [st.cfg.vkcfgMaxImagesInFlight];
	st.fbMain = new AnthemFramebuffer * [st.cfg.vkcfgMaxImagesInFlight];

	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.rd.createDescriptorPool(&st.descTgt[i]);
		st.rd.createColorAttachmentImage(&st.target[i], st.descTgt[i], 0, AT_IF_SWAPCHAIN, false, -1);
		st.rd.createSimpleFramebufferA(&st.fbMain[i], { st.target[i] }, st.passMain, st.depthMain);
	}

	AnthemDescriptorSetEntry dseCam{ st.descMain,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseImage{ st.descImage,AT_ACDS_SAMPLER,0 };
	st.rd.createGraphicsPipelineCustomized(&st.pipeMain, { dseCam,dseImage }, {}, st.passMain, st.sdMain, st.ldModel.getVertexBuffer(), &st.coptMain);

	st.cmdIdx = new uint32_t[st.cfg.vkcfgMaxImagesInFlight];
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.rd.drAllocateCommandBuffer(&st.cmdIdx[i]);
	}
	st.rd.createSemaphore(&st.drawComplete);
	st.rd.createFence(&st.drawReady);
}

void prepareTAA() {
	st.taa = std::make_unique<AnthemTAA>(&st.rd, 2);
	st.taa->addInput({ {st.descTgt[0],AT_ACDS_SAMPLER,0},{st.descTgt[1],AT_ACDS_SAMPLER,0} }, 0);
	st.taa->addInput({ {st.descTgt[1],AT_ACDS_SAMPLER,0},{st.descTgt[0],AT_ACDS_SAMPLER,0} }, 1);
	st.taa->prepare();
}

void recordCommand() {
	auto& r = st.rd;
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		ANTH_LOGI("Recording Command", i);
		r.drStartCommandRecording(st.cmdIdx[i]);

		// Write Image
		int k = (i - 1 + st.cfg.vkcfgMaxImagesInFlight) % st.cfg.vkcfgMaxImagesInFlight;
		r.drSetImageLayoutSimple(st.target[k], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, st.cmdIdx[i]);
		r.drSetSwapchainImageLayoutSimple(k, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, st.cmdIdx[i]);
		r.drCopySwapchainImageToImage(st.target[k], k, st.cmdIdx[i]);
		r.drSetSwapchainImageLayoutSimple(k, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, st.cmdIdx[i]);
		r.drSetImageLayoutSimple(st.target[k], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, st.cmdIdx[i]);


		// Main Pass

		r.drStartRenderPass(st.passMain, st.fbMain[i], st.cmdIdx[i], false);
		r.drSetViewportScissorFromSwapchain(st.cmdIdx[i]);
		r.drBindGraphicsPipeline(st.pipeMain, st.cmdIdx[i]);
		r.drBindVertexBuffer(st.ldModel.getVertexBuffer(), st.cmdIdx[i]);
		r.drBindIndexBuffer(st.ldModel.getIndexBuffer(), st.cmdIdx[i]);

		AnthemDescriptorSetEntry dseUniform{ st.descMain,AT_ACDS_UNIFORM_BUFFER,i };
		AnthemDescriptorSetEntry dseImage{ st.descImage,AT_ACDS_SAMPLER,0 };
		r.drBindDescriptorSetCustomizedGraphics({ dseUniform,dseImage }, st.pipeMain, st.cmdIdx[i]);
		r.drDrawIndexedIndirect(st.ldModel.getIndirectBuffer(), st.cmdIdx[i]);
		r.drEndRenderPass(st.cmdIdx[i]);


		r.drEndCommandRecording(st.cmdIdx[i]);
	}
}

void updateUniform() {
	AtMatf4 projOld,projNew, view, local;
	st.camera.getProjectionMatrix(projOld);
	st.camera.getProjectionMatrix(projNew);
	st.camera.getViewMatrix(view);

	local = AnthemLinAlg::axisAngleRotationTransform3<float, float>({ 0.0f,1.0f,0.0f },
		AT_PI * 0.5f);

	float pm[16], vm[16], lm[16],pmNew[16], lmNew[16];
	int oldHaltonIdx = (st.counter - 1 + 8) % 8;
	int curHaltonIdx = (st.counter) % 8;
	int scrW, scrH;
	st.rd.exGetWindowSize(scrH, scrW);

	projOld[0][2] += (st.halton[oldHaltonIdx].first * 2.0 - 1.0) / scrW;
	projOld[1][2] += (st.halton[oldHaltonIdx].second * 2.0 - 1.0) / scrH;

	projNew[0][2] += (st.halton[curHaltonIdx].first * 2.0 - 1.0) / scrW;
	projNew[1][2] += (st.halton[curHaltonIdx].second * 2.0 - 1.0) / scrH;

	projOld.columnMajorVectorization(pm);
	projNew.columnMajorVectorization(pmNew);

	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);

	local = AnthemLinAlg::axisAngleRotationTransform3<float, float>({ 0.0f,1.0f,0.0f },
		AT_PI * 0.75f);
	local.columnMajorVectorization(lmNew);

	st.uniStage->specifyUniforms(pm, vm, lm, pmNew, vm, lmNew);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniStage->updateBuffer(i);
	}
}

void mainLoop() {
	static int cur = 0;
	uint32_t imgIdx;
	updateUniform();
	st.drawReady->waitAndReset();
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.cmdIdx[cur], -1, {}, {}, st.drawReady, { st.drawComplete });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneralA(st.taa->getCommandIdx(cur), cur, { st.drawComplete },{ AT_SSW_COLOR_ATTACH_OUTPUT });
	st.rd.drPresentFrame(cur, cur);
	st.rd.forceCpuWaitDraw(cur);
	cur = 1 - cur;
	st.counter++;
}

int main() {
	initialize();
	prepareJitterSequence();
	loadModel();
	createMainStage();
	prepareTAA();
	st.rd.registerPipelineSubComponents();

	recordCommand();
	st.taa->recordCommand();

	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();

	return 0;
}