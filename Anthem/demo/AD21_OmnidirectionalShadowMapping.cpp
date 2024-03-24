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

#define ATRANGE(x,y) (std::views::iota((x),(y)))

struct Stage {
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemCamera camLight = AnthemCamera(AT_ACPT_PERSPECTIVE);

	std::vector<AnthemGLTFLoaderParseResult> model;
	AnthemSimpleModelIntegrator ldModel;

	AnthemShaderFilePaths spShadow;
	AnthemShaderModule* sdShadow;
	AnthemShaderFilePaths spMain;
	AnthemShaderModule* sdMain;

	AnthemRenderPass* passShadow;
	AnthemRenderPass* passMain;
	AnthemDepthBuffer* depthCubic;
	AnthemDepthBuffer* depthMain;

	AnthemFramebuffer* fbShadow;
	AnthemSwapchainFramebuffer* fbMain;

	AnthemGraphicsPipeline* pipeShadow;
	AnthemGraphicsPipeline* pipeMain;

	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uniStage;
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uniLight[6];
	AnthemDescriptorPool* descLight;
	AnthemDescriptorPool* descMain;
	AnthemDescriptorPool* descShadowMap;


	AnthemRenderPassSetupOption roptMain;
	AnthemGraphicsPipelineCreateProps coptMain;
	AnthemRenderPassSetupOption roptShadow;
	AnthemGraphicsPipelineCreateProps coptShadow;

	AnthemViewport* vpCubic;

	AnthemSemaphore* shadowMapped;
	AnthemFence* dbgFence;
	uint32_t cmdIdx[2];

	// Light Conf
	AnthemUniformBufferImpl<AtUniformVecf<4>, AtUniformVecf<4>>* uniLightConf;
	AnthemDescriptorPool* descLightConf;

	// Geometry 
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>, AtAttributeVecf<4>>* vxFloor;
	AnthemIndexBuffer* ixFloor;

	// Debug Pipeline
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>, AtAttributeVecf<4>>* sbox;
	AnthemIndexBuffer* ix;
	
	const bool debugEnable = false;
	const float lightX = 70.0f;

	AnthemDescriptorPool* descImage = nullptr;
	AnthemImage** image = nullptr;
}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "ptshadow\\ptshadow.";
	st += x;
	st += ".hlsl.spv";
	return st;
}

void initialize() {
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);

	st.camera.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 70, -100);
	if (st.debugEnable) {
		st.camera.specifyPosition(0, 0, 0);
	}
	st.camera.specifyFrontEyeRay(0, 0, 1);

	st.camLight.specifyFrustum((float)AT_PI  / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camLight.specifyPosition(70, 70, -150);
	st.camLight.specifyFrontEyeRay(0, 0, 1);

	st.rd.createSemaphore(&st.shadowMapped);
	st.rd.createFence(&st.dbgFence);

	st.rd.createDescriptorPool(&st.descLightConf);
	st.rd.createUniformBuffer(&st.uniLightConf, 0, st.descLightConf, -1);
	float lightPos[4] = { st.lightX,150,-100,1 };
	float ambient[4] =  { 0.1f,0.1f,0.1f,0.1f };
	st.uniLightConf->specifyUniforms(lightPos, ambient);
	for (auto i : ATRANGE(0, st.cfg.vkcfgMaxImagesInFlight)) {
		st.uniLightConf->updateBuffer(i);
	}

	st.rd.createDescriptorPool(&st.descMain);
	st.rd.createUniformBuffer(&st.uniStage, 0, st.descMain, -1);
}

void loadModel() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\klee\\untitled.gltf").c_str());
	loader.parseModel(config, st.model);
	st.model.pop_back();
	std::vector<AnthemUtlSimpleModelStruct> rp;
	for (auto& p : st.model)rp.push_back(p);
	st.ldModel.loadModel(&st.rd, rp, -1);

	st.rd.createDescriptorPool(&st.descImage);
	st.image = new AnthemImage * [st.model.size()];
	std::vector<AnthemImageContainer*> imgContainer;
	for (int i = 0; i < st.model.size(); i++) {
		AnthemImageLoader* loader = new AnthemImageLoader();
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

	st.rd.createDepthBuffer(&st.depthMain, false);
	st.rd.setupRenderPass(&st.passMain, &st.roptMain, st.depthMain);
	st.rd.createSwapchainImageFramebuffers(&st.fbMain, st.passMain, st.depthMain);

	AnthemDescriptorSetEntry dseCam{ st.descMain,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseSampler{ st.descShadowMap,AT_ACDS_SAMPLER,0 };
	AnthemDescriptorSetEntry dseLight{ st.descLight,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseLightConf{ st.descLightConf,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseImage{ st.descImage,AT_ACDS_SAMPLER,0 };
	st.rd.createGraphicsPipelineCustomized(&st.pipeMain, { dseCam,dseSampler,dseLight,dseLightConf,dseImage }, {}, st.passMain, st.sdMain, st.ldModel.getVertexBuffer(), &st.coptMain);

	for (auto i : ATRANGE(0, 2)) {
		st.rd.drAllocateCommandBuffer(&st.cmdIdx[i]);
	}
}

void createDebug() {
	const std::vector<std::array<float, 4>> vertices = {
		{-1,-1,-1,1},{1,-1,-1,1},{1,1,-1,1},{-1,1,-1,1},
		{-1,-1,1,1},{1,-1,1,1},{1,1,1,1},{-1,1,1,1}
	};
	const std::vector<uint32_t> indices = {
		0,1,2,2,3,0,
		4,5,6,6,7,4,
		0,3,7,7,4,0,
		1,2,6,6,5,1,
		0,1,5,5,4,0,
		3,2,6,6,7,3
	};
	st.rd.createVertexBuffer(&st.sbox);
	st.rd.createIndexBuffer(&st.ix);
	st.ix->setIndices(indices);
	st.sbox->setTotalVertices(8);
	for (auto i : ATRANGE(0, 8))st.sbox->insertData(i, vertices[i], vertices[i], vertices[i]);

	st.rd.createDescriptorPool(&st.descMain);
	st.rd.createUniformBuffer(&st.uniStage, 0, st.descMain, -1);

	st.spMain.fragmentShader = getShader("debug.frag");
	st.spMain.vertexShader = getShader("debug.vert");
	st.rd.createShader(&st.sdMain, &st.spMain);

	st.rd.createDepthBuffer(&st.depthMain, false);
	st.rd.setupRenderPass(&st.passMain, &st.roptMain, st.depthMain);
	st.rd.createSwapchainImageFramebuffers(&st.fbMain, st.passMain, st.depthMain);

	AnthemDescriptorSetEntry dseCam{ st.descMain,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseSampler{ st.descShadowMap,AT_ACDS_SAMPLER,0 };
	AnthemDescriptorSetEntry dseLight{ st.descLight,AT_ACDS_UNIFORM_BUFFER,0 };
	st.rd.createGraphicsPipelineCustomized(&st.pipeMain, { dseCam,dseSampler,dseLight }, {}, st.passMain, st.sdMain, st.ldModel.getVertexBuffer(), &st.coptMain);
	
	for (auto i : ATRANGE(0, 2)) {
		st.rd.drAllocateCommandBuffer(&st.cmdIdx[i]);
	}
}

void createShadowMap() {
	st.rd.createDescriptorPool(&st.descShadowMap);
	st.rd.createDescriptorPool(&st.descLight);
	std::vector<AnthemUniformBuffer*> unifs;
	for (auto i : ATRANGE(0,6)) {
		st.rd.createUniformBuffer(&st.uniLight[i], 0, nullptr, -1);
		unifs.push_back(st.uniLight[i]);
	}
	st.rd.addUniformBufferArrayToDescriptor(unifs, st.descLight, 0, -1);

	st.spShadow.fragmentShader = getShader("shadow.frag");
	st.spShadow.geometryShader = getShader("shadow.geom");
	st.spShadow.vertexShader = getShader("shadow.vert");

	st.rd.createShader(&st.sdShadow, &st.spShadow);
	st.rd.createDepthBufferCubicWithSampler(&st.depthCubic, st.descShadowMap,0, 4096, false);

	st.rd.createViewportCustom(&st.vpCubic, 4096, 4096, 0.0, 1.0);

	st.roptShadow.renderPassUsage = AT_ARPAA_DEPTH_STENCIL_ONLY_PASS;
	st.roptShadow.colorAttachmentFormats = {};
	st.rd.setupRenderPass(&st.passShadow, &st.roptShadow, st.depthCubic);
	st.rd.createSimpleFramebuffer(&st.fbShadow, nullptr, st.passShadow, st.depthCubic);

	AnthemDescriptorSetEntry dseLight{ st.descLight,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseUniform{ st.descMain,AT_ACDS_UNIFORM_BUFFER,0 };
	st.coptShadow.customViewport = st.vpCubic;
	st.rd.createGraphicsPipelineCustomized(&st.pipeShadow, { dseLight, dseUniform }, {}, st.passShadow, st.sdShadow, st.ldModel.getVertexBuffer(), &st.coptShadow);
}

void createGeometry(){
	st.rd.createVertexBuffer(&st.vxFloor);
	st.rd.createIndexBuffer(&st.ixFloor);
	st.vxFloor->setTotalVertices(4);
	float elevation = 5.0f;
	st.vxFloor->insertData(0, { -160,elevation,160,1 }, { 0,1,0,0 }, { 0,0,10,1 });
	st.vxFloor->insertData(1, { 160,elevation,160,1 }, { 0,1,0,0 }, { 0,0,10,1 });
	st.vxFloor->insertData(2, { 160,elevation,-160,1 }, { 0,1,0,0 }, { 0,0,10,1 });
	st.vxFloor->insertData(3, { -160,elevation,-160,1 }, { 0,1,0,0 }, { 0,0,10,1 });
	st.ixFloor->setIndices({ 0,1,2,2,3,0 });
}

void recordCommand() {
	auto& r = st.rd;
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		ANTH_LOGI("Recording Command", i);

		// Shadow Pass
		r.drStartCommandRecording(st.cmdIdx[i]);
		r.drStartRenderPass(st.passShadow, st.fbShadow, st.cmdIdx[i], false);
		r.drSetViewportScissor(st.vpCubic,st.cmdIdx[i]);
		r.drBindGraphicsPipeline(st.pipeShadow, st.cmdIdx[i]);
		r.drBindVertexBuffer(st.ldModel.getVertexBuffer(), st.cmdIdx[i]);
		r.drBindIndexBuffer(st.ldModel.getIndexBuffer(), st.cmdIdx[i]);
		AnthemDescriptorSetEntry dseLight{ st.descLight,AT_ACDS_UNIFORM_BUFFER,0 };
		AnthemDescriptorSetEntry dseUniform{ st.descMain,AT_ACDS_UNIFORM_BUFFER,0 };
		r.drBindDescriptorSetCustomizedGraphics({ dseLight,dseUniform }, st.pipeShadow, st.cmdIdx[i]);
		r.drDrawIndexedIndirect(st.ldModel.getIndirectBuffer(), st.cmdIdx[i]);

		r.drBindVertexBuffer(st.vxFloor, st.cmdIdx[i]);
		r.drBindIndexBuffer(st.ixFloor, st.cmdIdx[i]);
		r.drDraw(st.ixFloor->getIndexCount(), st.cmdIdx[i]);

		r.drEndRenderPass(st.cmdIdx[i]);
		r.drEndCommandRecording(st.cmdIdx[i]);

		// Main Pass
		r.drStartCommandRecording(i);
		r.drStartRenderPass(st.passMain,st.fbMain->getFramebufferObjectUnsafe(i), i, false);
		r.drSetViewportScissorFromSwapchain(i);
		r.drBindGraphicsPipeline(st.pipeMain, i);
		if (st.debugEnable) {
			r.drBindVertexBuffer(st.sbox, i);
			r.drBindIndexBuffer(st.ix, i);
			AnthemDescriptorSetEntry dseUniform{ st.descMain,AT_ACDS_UNIFORM_BUFFER,i };
			AnthemDescriptorSetEntry dseSampler{ st.descShadowMap,AT_ACDS_SAMPLER,i };
			AnthemDescriptorSetEntry dseLight{ st.descLight,AT_ACDS_UNIFORM_BUFFER,0 };

			r.drBindDescriptorSetCustomizedGraphics({ dseUniform,dseSampler,dseLight }, st.pipeMain, i);
			r.drDraw(st.ix->getIndexCount(), i);
		}
		else {
			r.drBindVertexBuffer(st.ldModel.getVertexBuffer(), i);
			r.drBindIndexBuffer(st.ldModel.getIndexBuffer(), i);
			AnthemDescriptorSetEntry dseUniform{ st.descMain,AT_ACDS_UNIFORM_BUFFER,i };
			AnthemDescriptorSetEntry dseSampler{ st.descShadowMap,AT_ACDS_SAMPLER,i };
			AnthemDescriptorSetEntry dseLight{ st.descLight,AT_ACDS_UNIFORM_BUFFER,0 };
			AnthemDescriptorSetEntry dseLightConf{ st.descLightConf,AT_ACDS_UNIFORM_BUFFER,0 };
			AnthemDescriptorSetEntry dseImage{ st.descImage,AT_ACDS_SAMPLER,0 };
			r.drBindDescriptorSetCustomizedGraphics({ dseUniform,dseSampler,dseLight,dseLightConf,dseImage }, st.pipeMain, i);
			r.drDrawIndexedIndirect(st.ldModel.getIndirectBuffer(), i);

			r.drBindVertexBuffer(st.vxFloor, i);
			r.drBindIndexBuffer(st.ixFloor, i);
			r.drDraw(st.ixFloor->getIndexCount(),i);

		}
		r.drEndRenderPass(i);
		r.drEndCommandRecording(i);
	}
}

void updateUniform() {

	AtMatf4 proj, view, local;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	local = AnthemLinAlg::axisAngleRotationTransform3<float, float>({0.0f,1.0f,0.0f},
		0.5f * static_cast<float>(glfwGetTime()));
	//local = AnthemLinAlg::eye<float, 4>();

	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);

	

	st.uniStage->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniStage->updateBuffer(i);
	}
}

void updateUniformLights() {
	std::vector<std::array<float, 3>> dirs = {
		{1,0,0},
		{-1,0,0},
		{0,1,0},
		{0,-1,0},
		{0,0,1},
		{0,0,-1}
	};
	std::vector<std::array<float, 3>> up = {
		{0,1,0},
		{0,1,0},
		{0,0,-1},
		{0,0,1}, 
		{0,1,0},
		{0,1,0},
	};
	float pm[6][16], vm[6][16], lm[6][16];
	for (auto k : ATRANGE(0, 6)) {
		AtMatf4 proj, view, local;
		st.camLight.specifyFrustum((float)AT_PI * 1.0 / 2.0f, 0.1f, 500.0f, 1.0f );
		st.camLight.specifyPosition(st.lightX, 150, -100);

		st.camLight.specifyFrontEyeRay(dirs[k][0], dirs[k][1], dirs[k][2]);
		st.camLight.specifyUp(up[k][0], up[k][1], up[k][2]);

		st.camLight.getProjectionMatrix(proj);
		st.camLight.getViewMatrix(view);
		local = AnthemLinAlg::eye<float, 4>();
		

		ANTH_LOGI(k);
		proj.print();

		proj.columnMajorVectorization(pm[k]);
		view.columnMajorVectorization(vm[k]);
		local.columnMajorVectorization(lm[k]);

		st.uniLight[k]->specifyUniforms(pm[k], vm[k], lm[k]);
		for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
			st.uniLight[k]->updateBuffer(i);
		}
	}
	
}

void mainLoop() {
	static int cur = -1;
	cur++;
	cur %= 2;
	uint32_t imgIdx;
	updateUniform();
	st.rd.drPrepareFrame(cur, &imgIdx);
	std::vector<const AnthemSemaphore*> shadowMapSemaToSignal = {st.shadowMapped};
	std::vector<AtSyncSemaphoreWaitStage> waitStages = { AtSyncSemaphoreWaitStage::AT_SSW_COLOR_ATTACH_OUTPUT };
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2(st.cmdIdx[cur], st.cmdIdx[cur], nullptr, nullptr, nullptr, &shadowMapSemaToSignal);
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral(cur, cur, &shadowMapSemaToSignal, &waitStages);
	st.rd.drPresentFrame(cur, cur);
}

int main() {
	initialize();
	loadModel();
	createShadowMap();
	createGeometry();
	if (!st.debugEnable) {
		createMainStage();
	}
	else {
		createDebug();
	}

	st.rd.registerPipelineSubComponents();
	updateUniformLights();

	recordCommand();
	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();
	
	return 0;
}