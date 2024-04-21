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

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::External;
using namespace Anthem::Core;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "stencil\\st.";
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
	AnthemSwapchainFramebuffer* fbMain;
	AnthemGraphicsPipeline* pipeMain;

	AnthemShaderFilePaths spOutline;
	AnthemShaderModule* sdOutline;
	AnthemRenderPassSetupOption roptOutline;
	AnthemGraphicsPipelineCreateProps coptOutline;
	AnthemRenderPass* passOutline;
	AnthemSwapchainFramebuffer* fbOutline;
	AnthemGraphicsPipeline* pipeOutline;

	AnthemDescriptorPool* descMain;
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uniStage;

	uint32_t* cmdIdx;
	AnthemSemaphore* drawComplete;
	AnthemFence* drawReady;
}st;

void initialize() {
	st.cfg.demoName = "26. Stencil Buffer / Object Outline";
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


void createMainStage() {
	st.spMain.fragmentShader = getShader("main.frag");
	st.spMain.vertexShader = getShader("main.vert");
	st.rd.createShader(&st.sdMain, &st.spMain);

	st.roptMain.clearDepthAttachmentOnLoad = true;
	st.roptMain.clearStencilAttachmentOnLoad = true;
	st.coptMain.enableDynamicStencilTesting = true;

	st.rd.createDepthStencilBuffer(&st.depthMain, false);
	st.rd.setupRenderPass(&st.passMain, &st.roptMain, st.depthMain);
	st.rd.createSwapchainImageFramebuffers(&st.fbMain, st.passMain, st.depthMain);
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

void createOutlineStage() {
	st.spOutline.fragmentShader = getShader("outline.frag");
	st.spOutline.vertexShader = getShader("outline.vert");
	st.spOutline.geometryShader = getShader("outline.geom");
	st.rd.createShader(&st.sdOutline, &st.spOutline);

	st.roptOutline.clearDepthAttachmentOnLoad = false;
	st.roptOutline.clearStencilAttachmentOnLoad = false;
	st.coptOutline.enableDynamicStencilTesting = true;
	st.coptOutline.enableDepthTesting = false;
	st.roptOutline.clearColorAttachmentOnLoad = { false };

	st.rd.setupRenderPass(&st.passOutline, &st.roptOutline, st.depthMain);
	st.rd.createSwapchainImageFramebuffers(&st.fbOutline, st.passOutline, st.depthMain);
	AnthemDescriptorSetEntry dseCam{ st.descMain,AT_ACDS_UNIFORM_BUFFER,0 };
	st.rd.createGraphicsPipelineCustomized(&st.pipeOutline, { dseCam }, {}, st.passOutline, st.sdOutline, st.ldModel.getVertexBuffer(), &st.coptOutline);
	

}

void recordCommand() {
	auto& r = st.rd;
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		ANTH_LOGI("Recording Command", i);
		
		// Main Pass
		r.drStartCommandRecording(st.cmdIdx[i]);
		r.drStartRenderPass(st.passMain, st.fbMain->getFramebufferObjectUnsafe(i), st.cmdIdx[i], false);
		r.drSetViewportScissorFromSwapchain(st.cmdIdx[i]);
		r.drBindGraphicsPipeline(st.pipeMain, st.cmdIdx[i]);
		r.drBindVertexBuffer(st.ldModel.getVertexBuffer(), st.cmdIdx[i]);
		r.drBindIndexBuffer(st.ldModel.getIndexBuffer(), st.cmdIdx[i]);

		AnthemDescriptorSetEntry dseUniform{ st.descMain,AT_ACDS_UNIFORM_BUFFER,i };
		AnthemDescriptorSetEntry dseImage{ st.descImage,AT_ACDS_SAMPLER,0 };
		r.drBindDescriptorSetCustomizedGraphics({ dseUniform,dseImage }, st.pipeMain, st.cmdIdx[i]);
		r.drSetStencilOp(VK_STENCIL_OP_KEEP,VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS,1, st.cmdIdx[i]);
		r.drDrawIndexedIndirect(st.ldModel.getIndirectBuffer(), st.cmdIdx[i]);
		r.drEndRenderPass(st.cmdIdx[i]);
		r.drEndCommandRecording(st.cmdIdx[i]);

		// Outline Pass
		r.drStartCommandRecording(i);
		r.drStartRenderPass(st.passOutline, st.fbOutline->getFramebufferObjectUnsafe(i), i, false);
		r.drSetViewportScissorFromSwapchain(i);
		r.drBindGraphicsPipeline(st.pipeOutline, i);
		r.drBindVertexBuffer(st.ldModel.getVertexBuffer(), i);
		r.drBindIndexBuffer(st.ldModel.getIndexBuffer(), i);

		AnthemDescriptorSetEntry dseUniform2{ st.descMain,AT_ACDS_UNIFORM_BUFFER,i };
		r.drBindDescriptorSetCustomizedGraphics({ dseUniform2 }, st.pipeOutline, i);
		r.drSetStencilOp(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NOT_EQUAL, 1, i);
		r.drDrawIndexedIndirect(st.ldModel.getIndirectBuffer(), i);
		r.drEndRenderPass(i);
		r.drEndCommandRecording(i);
	}
}

void updateUniform() {
	AtMatf4 proj, view, local;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	local = AnthemLinAlg::axisAngleRotationTransform3<float, float>({ 0.0f,1.0f,0.0f },
		0.5f * static_cast<float>(glfwGetTime()));

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
	ANTH_LOGI(cur);
	updateUniform();
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.cmdIdx[cur], -1, {}, {}, nullptr, { st.drawComplete });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneralA(cur, cur, { st.drawComplete }, { AtSyncSemaphoreWaitStage::AT_SSW_COLOR_ATTACH_OUTPUT });
	st.rd.drPresentFrame(cur, cur);
}

int main() {
	initialize();
	loadModel();
	createMainStage();
	createOutlineStage();
	st.rd.registerPipelineSubComponents();

	recordCommand();
	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();

	return 0;
}