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
#include "../include/components/postprocessing/AnthemMLAAEdge.h"
#include "../include/components/postprocessing/AnthemMLAABlend.h"
#include "../include/components/postprocessing/AnthemMLAAMix.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::Components::Postprocessing;
using namespace Anthem::External;
using namespace Anthem::Core;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "mlaa\\mlaademo.";
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
	AnthemFramebuffer* fbMain;
	AnthemGraphicsPipeline* pipeMain;

	AnthemDescriptorPool* descMain;
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uniStage;

	AnthemDescriptorPool* descTgt;
	AnthemImage* target;

	uint32_t* cmdIdx;
	AnthemSemaphore* drawComplete;
	AnthemSemaphore* mlaaThDone;
	AnthemSemaphore* mlaaBlendDone;

	AnthemFence* drawReady;
	std::vector<std::string> requiredTex;

	std::unique_ptr<AnthemMLAAEdge> mlaaThreshold;
	std::unique_ptr<AnthemMLAABlend> mlaaBlend;
	std::unique_ptr<AnthemMLAAMix> mlaaMix;

	AnthemDescriptorPool* descBlendFactor;
	AnthemImage* blendFactor;
}st;

void initialize() {
	st.cfg.demoName = "27-C. Morphological Anti-Aliasing";
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);

	st.camera.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 0.9, -1.3);
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
	st.requiredTex = st.ldModel.getRequiredTextures();

	st.rd.createDescriptorPool(&st.descImage);

	st.image = new AnthemImage * [st.requiredTex.size()];
	std::vector<AnthemImageContainer*> imgContainer;
	for (int i = 0; i < st.requiredTex.size(); i++) {
		std::unique_ptr<AnthemImageLoader> loader = std::make_unique<AnthemImageLoader>();
		uint32_t texWidth, texHeight, texChannels;
		uint8_t* texData;

		std::string texPath = st.model[i].basePath + st.requiredTex[i];
		if (st.requiredTex[i] == "") {
			texPath = ANTH_ASSET_DIR;
			texPath += "cat.jpg";
		}
		loader->loadImage(texPath.c_str(), &texWidth, &texHeight, &texChannels, &texData);
		st.rd.createTexture(&st.image[i], st.descImage, texData, texWidth, texHeight, texChannels, 0, false, false, AT_IF_SRGB_UINT8, -1, true);
		imgContainer.push_back(st.image[i]);
	}
	st.rd.addSamplerArrayToDescriptor(imgContainer, st.descImage, 0, -1);
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

	st.rd.createDescriptorPool(&st.descTgt);
	st.rd.createColorAttachmentImage(&st.target, st.descTgt, 0, AT_IF_SBGR_UINT8, false, -1);

	st.rd.createSimpleFramebufferA(&st.fbMain, { st.target }, st.passMain, st.depthMain);
	AnthemDescriptorSetEntry dseCam{ st.descMain,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseImage{ st.descImage,AT_ACDS_SAMPLER,0 };
	st.rd.createGraphicsPipelineCustomized(&st.pipeMain, { dseCam,dseImage }, {}, st.passMain, st.sdMain, st.ldModel.getVertexBuffer(), &st.coptMain);

	st.cmdIdx = new uint32_t[st.cfg.vkcfgMaxImagesInFlight];
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.rd.drAllocateCommandBuffer(&st.cmdIdx[i]);
	}
	st.rd.createSemaphore(&st.drawComplete);
	st.rd.createSemaphore(&st.mlaaThDone);
	st.rd.createSemaphore(&st.mlaaBlendDone);
	st.rd.createFence(&st.drawReady);
}

void prepareMLAA() {
	st.mlaaThreshold = std::make_unique<AnthemMLAAEdge>((AnthemSimpleToyRenderer*)&st.rd, 2,16);
	st.mlaaThreshold->addInput({ {st.descTgt,AT_ACDS_SAMPLER,0} });
	st.mlaaThreshold->prepare(true);

	st.mlaaBlend = std::make_unique<AnthemMLAABlend>((AnthemSimpleToyRenderer*)&st.rd, 2, 16);
	st.mlaaBlend->prepareBlendTexture(std::string(ANTH_ASSET_TEMP_DIR) + "/mlaa16.png");
	st.rd.createDescriptorPool(&st.descBlendFactor);
	std::unique_ptr<AnthemImageLoader> loader = std::make_unique<AnthemImageLoader>();
	uint32_t texWidth, texHeight, texChannels;
	uint8_t* texData;
	loader->loadImage(ANTH_ASSET_TEMP_DIR"/mlaa16.png", &texWidth, &texHeight, &texChannels, &texData);
	st.rd.createTexture(&st.blendFactor, st.descBlendFactor, texData, texWidth, texHeight, texChannels, 0, false, false);

	st.mlaaBlend->addInput({
		{st.mlaaThreshold->getColorAttachmentDescId(0),AT_ACDS_SAMPLER,0},
		{st.descBlendFactor,AT_ACDS_SAMPLER,0}
	},0);
	st.mlaaBlend->addInput({
		{st.mlaaThreshold->getColorAttachmentDescId(1),AT_ACDS_SAMPLER,0},
		{st.descBlendFactor,AT_ACDS_SAMPLER,0}
	}, 1);
	st.mlaaBlend->prepare(true);

	st.mlaaMix = std::make_unique<AnthemMLAAMix>((AnthemSimpleToyRenderer*)&st.rd, 2, 16);
	st.mlaaMix->addInput({
		{st.mlaaBlend->getColorAttachmentDescId(0),AT_ACDS_SAMPLER,0},
		{st.descTgt,AT_ACDS_SAMPLER,0} 
	},0);
	st.mlaaMix->addInput({
		{st.mlaaBlend->getColorAttachmentDescId(1),AT_ACDS_SAMPLER,0},
		{st.descTgt,AT_ACDS_SAMPLER,0}
	}, 1);
	st.mlaaMix->prepare();
}

void recordCommand() {
	auto& r = st.rd;
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		ANTH_LOGI("Recording Command", i);

		// Main Pass
		r.drStartCommandRecording(st.cmdIdx[i]);
		r.drStartRenderPass(st.passMain, st.fbMain, st.cmdIdx[i], false);
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
	AtMatf4 proj, view, local;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	local = AnthemLinAlg::axisAngleRotationTransform3<float, float>({ 0.0f,1.0f,0.0f },
		AT_PI * 1.25);

	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);

	st.uniStage->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniStage->updateBuffer(i);
	}
}

void mainLoop() {
	static int cur = -1;
	cur++;
	cur %= 2;
	uint32_t imgIdx;
	updateUniform();
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.cmdIdx[cur], -1, {}, {}, nullptr, { st.drawComplete });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.mlaaThreshold->getCommandIdx(cur), -1,
		{st.drawComplete }, { AT_SSW_COLOR_ATTACH_OUTPUT }, nullptr, { st.mlaaThDone });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.mlaaBlend->getCommandIdx(cur), -1,
		{ st.mlaaThDone }, { AT_SSW_COLOR_ATTACH_OUTPUT }, nullptr, { st.mlaaBlendDone });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneralA(st.mlaaMix->getCommandIdx(cur), cur,
		{ st.mlaaBlendDone },{ AT_SSW_COLOR_ATTACH_OUTPUT });
	st.rd.drPresentFrame(cur, cur);
}

int main() {
	initialize();
	loadModel();
	createMainStage();
	prepareMLAA();
	st.rd.registerPipelineSubComponents();

	recordCommand();
	st.mlaaThreshold->recordCommandOffscreen();
	st.mlaaBlend->recordCommandOffscreen();
	st.mlaaMix->recordCommand();

	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();

	return 0;
}