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
#include "../include/components/passhelper/AnthemPassHelper.h"
#include "../include/components/passhelper/AnthemComputePassHelper.h"
#include "../include/components/postprocessing/AnthemPostIdentity.h"
#include "../include/components/math/AnthemLowDiscrepancySequence.h"
#include "../include/components/postprocessing/AnthemFXAA.h"
using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::Components::PassHelper;
using namespace Anthem::Components::Postprocessing;
using namespace Anthem::Components::Math;
using namespace Anthem::External;
using namespace Anthem::Core;

struct Stage {
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;

	AnthemCamera camMain = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemCamera camLight = AnthemCamera(AT_ACPT_ORTHO);
	AnthemFrameRateMeter fpsMeter = AnthemFrameRateMeter(10);


	std::vector<AnthemGLTFLoaderParseResult> gltfModel;
	AnthemGLTFLoaderTexParseResult gltfTex;
	AnthemSimpleModelIntegrator model;
	AnthemDescriptorPool* descPbrBaseTex = nullptr;
	AnthemImage** pbrBaseTex = nullptr;
	std::vector<std::string> requiredTex;

	AnthemShaderFilePaths spMain;
	AnthemShaderFilePaths spShadow;

	AnthemDescriptorPool* descUniCamMain;
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uniCamMain;

	AnthemDescriptorPool* descUniCamLight;
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uniCamLight;

	AnthemDescriptorPool* descUniFogConsts;
	AnthemUniformBufferImpl<
		AtUniformVeci<4>,	//Fog Size
		AtUniformVecf<4>,   //Density & Anisotropy & Jitter & FirstRW
		AtUniformVecf<4>,   //LightColor & Ambient
		AtUniformVecf<4>,	//Light Direction
		AtUniformVecf<4>,	//zFar,zNear
		AtUniformVecf<4>,	//Cam Pos
		AtUniformVecf<4>,	//Jitter
		AtUniformMatf<4>	//Inverse VP
	>* uniFogConsts;
	
	std::unique_ptr<AnthemPassHelper> mainPass;
	std::unique_ptr<AnthemPassHelper> shadowPass;
	std::unique_ptr<AnthemPostIdentity> idPass;
	std::unique_ptr<AnthemPassHelper> deferPass;
	std::unique_ptr<AnthemPassHelper> aoPass;
	std::unique_ptr<AnthemComputePassHelper> lightPartPass;
	std::unique_ptr<AnthemComputePassHelper> rayMarchPass;
	std::unique_ptr<AnthemFXAA> fxaaPass;

	std::function<void(int, int, int, int)> keyController;

	AnthemSemaphore* shadowComplete;
	AnthemSemaphore* lightScatAttrComplete;
	AnthemSemaphore* rayMarchComplete;
	AnthemSemaphore* mainComplete;
	AnthemSemaphore* deferComplete;
	AnthemSemaphore* aoComplete;
	AnthemSemaphore* aaComplete;
	AnthemFence* fence;

	AnthemImage** lightScatterVolume;
	AnthemDescriptorPool** descLightScatVol;

	AnthemImage* fogVolume;
	AnthemDescriptorPool* descFogVolume;
	AnthemDescriptorPool* descFogVolumeSampler;
	float firstRW = 1.0;

	AnthemHaltonSequence hseq1 = AnthemHaltonSequence(2, 16);
	AnthemHaltonSequence hseq2 = AnthemHaltonSequence(3, 16);
	AnthemHaltonSequence hseq3 = AnthemHaltonSequence(5, 16);

	AnthemDescriptorPool* descMainTarget;
	AnthemImage* mainTarget;

	using uxSamples = AtUniformVecfArray<4, 64>;
	AnthemDescriptorPool* descAOSamples;
	AnthemUniformBufferImpl<uxSamples>* aoSamples;
	int round = 0;
}st;


struct GBuffer {
	AnthemDescriptorPool* dPos;
	AnthemDescriptorPool* dNorm;
	AnthemDescriptorPool* dBaseColor;
	AnthemDescriptorPool* dAO;
	AnthemDescriptorPool* dNdc;
	AnthemDescriptorPool* dTangent;

	AnthemImage* pos;
	AnthemImage* norm;
	AnthemImage* baseColor;
	AnthemImage* ao;
	AnthemImage* ndc;
	AnthemImage* tangent;
}gb;

struct DeferCanvas {
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>>* vx;
	AnthemIndexBuffer* ix;
}canvas;

#define DCONST static constexpr const
struct StageConstants {
	DCONST uint32_t VOLSIZE_X = 400;
	DCONST uint32_t VOLSIZE_Y = 256;
	DCONST uint32_t VOLSIZE_Z = 400;
	float FOG_DENSITY = 0.002f;
	DCONST float ANISOTROPY = 0.0f;
	DCONST float LIGHT_R = 1.0f;
	DCONST float LIGHT_G = 1.0f;
	DCONST float LIGHT_B = 1.0f;
	float LIGHT_I = 40.0f;
	float AMBIENT_I = 0.05f;

	DCONST uint32_t THREAD_X = 8;
	DCONST uint32_t THREAD_Y = 8;
	DCONST uint32_t THREAD_Z = 8;

	DCONST float LIGHTDIR_X = 0;
	DCONST float LIGHTDIR_Y = -1.7;
	DCONST float LIGHTDIR_Z = 1;

	DCONST float Z_NEAR = 0.1f;
	DCONST float Z_FAR = 2500.0f;
	DCONST float Z_FAR_SHADOW = 4000.0f;

	DCONST float CAM_INIT_POS_X = 200;
	DCONST float CAM_INIT_POS_Y = 550;
	DCONST float CAM_INIT_POS_Z = 420;

}sc;
#undef DCONST

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "volfog\\volfog.";
	st += x;
	st += ".hlsl.spv";
	return st;
}

void initialize() {
	st.cfg.demoName = "28. Voxel-based Volumetric Fog";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);

	st.camMain.specifyFrustum((float)AT_PI * 1.0f / 2.0f, sc.Z_NEAR, sc.Z_FAR, 1.0f * rdW / rdH);
	st.camMain.specifyFrontEyeRay(1, 0, -0.5);
	st.camMain.specifyPosition(sc.CAM_INIT_POS_X, sc.CAM_INIT_POS_Y, sc.CAM_INIT_POS_Z);

	st.camLight.specifyOrthoClipSpace(sc.Z_NEAR, sc.Z_FAR_SHADOW, 1.0f * rdW / rdH, 250.0f);
	st.camLight.specifyPosition(0, 1800, -550);
	st.camLight.specifyFrontEyeRay(sc.LIGHTDIR_X, sc.LIGHTDIR_Y, sc.LIGHTDIR_Z);
	st.camLight.specifyUp(0, 1, 0);

	st.rd.createDescriptorPool(&st.descUniCamMain);
	st.rd.createUniformBuffer(&st.uniCamMain, 0, st.descUniCamMain, -1);

	st.rd.createDescriptorPool(&st.descUniCamLight);
	st.rd.createUniformBuffer(&st.uniCamLight, 0, st.descUniCamLight, -1);

	st.rd.createDescriptorPool(&st.descUniFogConsts);
	st.rd.createUniformBuffer(&st.uniFogConsts, 0, st.descUniFogConsts, -1);

	st.rd.createDescriptorPool(&st.descAOSamples);
	st.rd.createUniformBuffer(&st.aoSamples, 0, st.descAOSamples, -1);

	st.keyController = st.camMain.getKeyboardController();
	//st.rd.ctSetKeyBoardController(st.keyController);

	st.rd.createSemaphore(&st.shadowComplete);
	st.rd.createSemaphore(&st.rayMarchComplete);
	st.rd.createSemaphore(&st.lightScatAttrComplete);
	st.rd.createSemaphore(&st.mainComplete);
	st.rd.createSemaphore(&st.deferComplete);
	st.rd.createSemaphore(&st.aoComplete);
	st.rd.createSemaphore(&st.aaComplete);
	st.rd.createFence(&st.fence);
}

void loadModel() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\sponza\\glTF\\Sponza.gltf").c_str());
	loader.parseModel(config, st.gltfModel, &st.gltfTex);
	std::vector<AnthemUtlSimpleModelStruct> rp;
	for (auto& p : st.gltfModel)rp.push_back(p);
	st.model.loadModel(&st.rd, rp, -1);
	st.requiredTex = st.model.getRequiredTextures(); 

	st.rd.createDescriptorPool(&st.descPbrBaseTex);

	int numTexs = st.gltfTex.tex.size();
	st.pbrBaseTex = new AnthemImage * [numTexs];
	std::vector<AnthemImageContainer*> imgContainer;
	for (int i = 0; i < numTexs; i++) {
		st.rd.createTexture(&st.pbrBaseTex[i], st.descPbrBaseTex, st.gltfTex.tex[i].data(), st.gltfTex.width[i],
			st.gltfTex.height[i], st.gltfTex.channels[i], 0, false, false, AT_IF_UNORM_UINT8, -1, true);
		imgContainer.push_back(st.pbrBaseTex[i]);
	}
	st.rd.addSamplerArrayToDescriptor(imgContainer, st.descPbrBaseTex, 0, -1);
}

void prepareAOSamples() {
	std::vector<float> samples(4 * 64);
	for (int i = 0; i < 4 * 64; i+=4) {
		const auto d = AnthemLinAlg::randomVector3<float>();
		samples[i] = d[0];
		samples[i + 1] = d[1];
		samples[i + 2] = d[2];
		samples[i + 3] = (rand()%30);
	}
	st.aoSamples->specifyUniforms(samples.data());
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.aoSamples->updateBuffer(i);
	}
}

void prepareCanvas() {
	st.rd.createVertexBuffer(&canvas.vx);
	st.rd.createIndexBuffer(&canvas.ix);
	canvas.vx->setTotalVertices(6);
	canvas.vx->insertData(0, { -1,-1,0,1 }, { 0,0,0,0 });
	canvas.vx->insertData(1, { 1,-1,0,1 }, { 1,0,0,0 });
	canvas.vx->insertData(2, { 1,1,0,1 }, { 1,1,0,0 });
	canvas.vx->insertData(3, { -1,1,0,1 }, { 0,1,0,0 });
	canvas.ix->setIndices({ 0,1,2,2,3,0 });
}
void prepareDefer() {
	st.rd.createDescriptorPool(&gb.dAO);
	st.rd.createDescriptorPool(&gb.dBaseColor);
	st.rd.createDescriptorPool(&gb.dNorm);
	st.rd.createDescriptorPool(&gb.dPos);
	st.rd.createDescriptorPool(&gb.dNdc);
	st.rd.createDescriptorPool(&gb.dTangent);

	st.rd.createColorAttachmentImage(&gb.ao, gb.dAO, 0, AT_IF_SIGNED_FLOAT32, false, 0);
	st.rd.createColorAttachmentImage(&gb.baseColor, gb.dBaseColor, 0, AT_IF_SIGNED_FLOAT32, false, 0);
	st.rd.createColorAttachmentImage(&gb.pos, gb.dPos, 0, AT_IF_SIGNED_FLOAT32, false, 0);
	st.rd.createColorAttachmentImage(&gb.norm, gb.dNorm, 0, AT_IF_SIGNED_FLOAT32, false, 0);
	st.rd.createColorAttachmentImage(&gb.ndc, gb.dNdc, 0, AT_IF_SIGNED_FLOAT32, false, 0);
	st.rd.createColorAttachmentImage(&gb.tangent, gb.dTangent, 0, AT_IF_SIGNED_FLOAT32, false, 0);

	st.deferPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.deferPass->shaderPath.vertexShader = getShader("defer.vert");
	st.deferPass->shaderPath.fragmentShader = getShader("defer.frag");
	st.deferPass->vxLayout = st.model.getVertexBuffer();
	st.deferPass->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.deferPass->passOpt.clearColorAttachmentOnLoad = { true,true,true,true,true };
	st.deferPass->passOpt.clearColors = { {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0} };
	st.deferPass->pipeOpt.blendPreset = { AT_ABP_NO_BLEND,AT_ABP_NO_BLEND ,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
	st.deferPass->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32 };
	st.deferPass->setRenderTargets({ gb.baseColor,gb.norm,gb.pos,gb.ndc,gb.tangent });

	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.deferPass->setDescriptorLayouts({
			{st.descUniCamMain,AT_ACDS_UNIFORM_BUFFER,0},
			{st.descPbrBaseTex,AT_ACDS_SAMPLER,0},
		}, i);
	}
	st.deferPass->buildGraphicsPipeline();

	st.aoPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.aoPass->shaderPath.vertexShader = getShader("ao.vert");
	st.aoPass->shaderPath.fragmentShader = getShader("ao.frag");
	st.aoPass->vxLayout = canvas.vx;
	st.aoPass->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.aoPass->passOpt.clearColorAttachmentOnLoad = { true };
	st.aoPass->passOpt.clearColors = { {0, 0, 0, 0}, };
	st.aoPass->pipeOpt.blendPreset = { AT_ABP_NO_BLEND };
	st.aoPass->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.aoPass->setRenderTargets({ gb.ao });
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.aoPass->setDescriptorLayouts({
			{st.descAOSamples,AT_ACDS_UNIFORM_BUFFER,0},
			{gb.dBaseColor,AT_ACDS_SAMPLER,0},
			{gb.dNorm,AT_ACDS_SAMPLER,0},
			{gb.dPos,AT_ACDS_SAMPLER,0},
			{gb.dNdc,AT_ACDS_SAMPLER,0},
			{gb.dTangent,AT_ACDS_SAMPLER,0},
			{st.descUniCamMain,AT_ACDS_UNIFORM_BUFFER,0}
		}, i);
	}
	st.aoPass->buildGraphicsPipeline();
}
void prepareMain() {
	st.rd.createDescriptorPool(&st.descMainTarget);
	st.rd.createColorAttachmentImage(&st.mainTarget, st.descMainTarget, 0, AT_IF_SIGNED_FLOAT32, false, 0);

	st.mainPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.mainPass->shaderPath.vertexShader = getShader("main.vert");
	st.mainPass->shaderPath.fragmentShader = getShader("main.frag");
	st.mainPass->vxLayout = canvas.vx;
	st.mainPass->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.mainPass->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.mainPass->setRenderTargets({ st.mainTarget });
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.mainPass->setDescriptorLayouts({
			{st.descUniCamMain,AT_ACDS_UNIFORM_BUFFER,0},
			{st.descUniCamLight,AT_ACDS_UNIFORM_BUFFER,0},
			{st.shadowPass->getDepthDescriptor(i),AT_ACDS_SAMPLER,0},
			{st.descFogVolumeSampler,AT_ACDS_SAMPLER,0},
			{st.descUniFogConsts,AT_ACDS_UNIFORM_BUFFER,0},
			{gb.dBaseColor,AT_ACDS_SAMPLER,0},
			{gb.dNorm,AT_ACDS_SAMPLER,0},
			{gb.dPos,AT_ACDS_SAMPLER,0},
			{gb.dNdc,AT_ACDS_SAMPLER,0},
			{gb.dAO,AT_ACDS_SAMPLER,0},
		},i);
	}
	
	st.mainPass->buildGraphicsPipeline();
}

void preparePostAA() {
	st.fxaaPass = std::make_unique<AnthemFXAA>(&st.rd, 2);
	st.fxaaPass->addInput({
		{st.descMainTarget,AT_ACDS_SAMPLER,0},
		});
	st.fxaaPass->prepare();
}

void prepareShadow() {
	st.shadowPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.shadowPass->shaderPath.vertexShader = getShader("shadow.vert");
	st.shadowPass->enableDepthSampler = true;
	st.shadowPass->passOpt.colorAttachmentFormats = {};
	st.shadowPass->passOpt.renderPassUsage = AT_ARPAA_DEPTH_STENCIL_ONLY_PASS;
	st.shadowPass->vxLayout = st.model.getVertexBuffer();
	st.shadowPass->setDescriptorLayouts({
		{st.descUniCamLight,AT_ACDS_UNIFORM_BUFFER,0},
	});
	st.shadowPass->buildGraphicsPipeline();
}

void prepareFogVolume() {
	st.descLightScatVol = new AnthemDescriptorPool * [st.cfg.vkcfgMaxImagesInFlight];
	st.lightScatterVolume = new AnthemImage * [st.cfg.vkcfgMaxImagesInFlight];
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.rd.createDescriptorPool(&st.descLightScatVol[i]);
		st.rd.createTexture3d(&st.lightScatterVolume[i], nullptr, nullptr, sc.VOLSIZE_X, sc.VOLSIZE_Y, sc.VOLSIZE_Z,
			4, 0, AT_IF_SIGNED_FLOAT32, -1, AT_IU_COMPUTE_OUTPUT);
		st.lightScatterVolume[i]->toGeneralLayout();
		st.rd.addStorageImageArrayToDescriptor({ st.lightScatterVolume[i] }, st.descLightScatVol[i], 0, -1);
	}
	st.rd.createDescriptorPool(&st.descFogVolume);
	st.rd.createDescriptorPool(&st.descFogVolumeSampler);
	st.rd.createTexture3d(&st.fogVolume,st.descFogVolumeSampler, nullptr, sc.VOLSIZE_X, sc.VOLSIZE_Y, sc.VOLSIZE_Z,
		4, 0, AT_IF_SIGNED_FLOAT32, -1, AT_IU_COMPUTE_OUTPUT);
	st.fogVolume->toGeneralLayout();
	st.rd.addStorageImageArrayToDescriptor({ st.fogVolume }, st.descFogVolume, 0, -1);
}

void prepareLightParticipationPass() {
	st.lightPartPass = std::make_unique<AnthemComputePassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.lightPartPass->shaderPath.computeShader = getShader("lightpart.comp");
	st.lightPartPass->workGroupSize = { sc.VOLSIZE_X / sc.THREAD_X, sc.VOLSIZE_Y / sc.THREAD_Y, sc.VOLSIZE_Z / sc.THREAD_Z };
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.lightPartPass->setDescriptorLayouts({
			{st.descUniFogConsts,AT_ACDS_UNIFORM_BUFFER,0},
			{st.descLightScatVol[i],AT_ACDS_STORAGE_IMAGE,0},
			{st.shadowPass->getDepthDescriptor(i),AT_ACDS_SAMPLER,0},
			{st.descUniCamLight,AT_ACDS_UNIFORM_BUFFER,0},
			{st.descLightScatVol[1-i],AT_ACDS_STORAGE_IMAGE,0},
		},i);
	}
	st.lightPartPass->buildComputePipeline();
}

void prepareRayMarchingPass() {
	st.rayMarchPass = std::make_unique<AnthemComputePassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.rayMarchPass->shaderPath.computeShader = getShader("raymarch.comp");
	st.rayMarchPass->workGroupSize = { sc.VOLSIZE_X / sc.THREAD_X, sc.VOLSIZE_Y / sc.THREAD_Y, 1 };
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.rayMarchPass->setDescriptorLayouts({
			{st.descUniFogConsts,AT_ACDS_UNIFORM_BUFFER,0},
			{st.descFogVolume,AT_ACDS_STORAGE_IMAGE,0},
			{st.descLightScatVol[i],AT_ACDS_STORAGE_IMAGE,0},
		},i);
	}
	st.rayMarchPass->buildComputePipeline();
}

void prepareIdPass() {
	st.idPass = std::make_unique<AnthemPostIdentity>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	//st.idPass->addInput({ {st.shadowPass->getDepthDescriptor(0),AT_ACDS_SAMPLER,0} }, 0);
	//st.idPass->addInput({ {st.shadowPass->getDepthDescriptor(1),AT_ACDS_SAMPLER,0} }, 1);
	st.idPass->addInput({ { gb.dAO,AT_ACDS_SAMPLER,0} });
	st.idPass->prepare();
}


void recordCommand() {
	st.mainPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(canvas.vx, x);
		st.rd.drBindIndexBuffer(canvas.ix, x);
		st.rd.drDraw(6, x);
	});
	st.aoPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(canvas.vx, x);
		st.rd.drBindIndexBuffer(canvas.ix, x);
		st.rd.drDraw(6, x);
	});
	st.deferPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.model.getVertexBuffer(), x);
		st.rd.drBindIndexBuffer(st.model.getIndexBuffer(), x);
		st.rd.drDrawIndexedIndirect(st.model.getIndirectBuffer(), x);
	});
	st.shadowPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.model.getVertexBuffer(), x);
		st.rd.drBindIndexBuffer(st.model.getIndexBuffer(), x);
		st.rd.drDrawIndexedIndirect(st.model.getIndirectBuffer(), x);
	});
	st.idPass->recordCommand();
	st.rayMarchPass->recordCommand();
	st.lightPartPass->recordCommand();
	st.fxaaPass->recordCommand();
}

void updateUniform() {
	// Common Camera
	AtMatf4 proj, view, local, pv;
	st.camMain.getProjectionMatrix(proj);
	st.camMain.getViewMatrix(view);
	local = AnthemLinAlg::axisAngleRotationTransform3<float, float>({ 0.0f,1.0f,0.0f }, glfwGetTime() * 0.00f);

	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);
	pv = proj.multiply(view);
	auto invPv = AnthemLinAlg::gaussJordan(proj.multiply(view));
	st.uniCamMain->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniCamMain->updateBuffer(i);
	}

	st.camLight.getProjectionMatrix(proj);
	st.camLight.getViewMatrix(view);
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);

	st.uniCamLight->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniCamLight->updateBuffer(i);
	}

	// For CS
	float jitter = AnthemLinAlg::randomNumber<float>() - 0.5;
	float invPvRaw[16];
	float lightColor[4] = { sc.LIGHT_R * sc.LIGHT_I,sc.LIGHT_G * sc.LIGHT_I,sc.LIGHT_B * sc.LIGHT_I,sc.AMBIENT_I };
	int fogSize[4] = { sc.VOLSIZE_X,sc.VOLSIZE_Y,sc.VOLSIZE_Z,0 };
	float lightAttr[4] = { sc.FOG_DENSITY,sc.ANISOTROPY,jitter,st.firstRW };
	float lightDir[4] = { sc.LIGHTDIR_X,sc.LIGHTDIR_Y,sc.LIGHTDIR_Z,0 };
	float farNear[4] = { sc.Z_FAR,sc.Z_NEAR,0,0 };
	float jitterSeq[4] = { 0,0 ,st.hseq1[st.round] * 0.05 - 0.5 ,0 };
	ANTH_TODO("Dynamic camera");
	float camPos[4] = { sc.CAM_INIT_POS_X,sc.CAM_INIT_POS_Y,sc.CAM_INIT_POS_Z,0 };

	invPv.columnMajorVectorization(invPvRaw);
	st.uniFogConsts->specifyUniforms(fogSize, lightAttr, lightColor, lightDir, farNear, camPos, jitterSeq, invPvRaw);
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.uniFogConsts->updateBuffer(i);
	}
	st.firstRW = 0;
	st.round = st.round + 1;
	st.round %= 8;
}
void setupImgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.FontGlobalScale = 1.8;
	ImGui::StyleColorsDark();
	st.rd.exInitImGui();
}
void prepareImguiFrame() {
	st.fpsMeter.record();
	std::stringstream ss;
	ss << "FPS:";
	ss << st.fpsMeter.getFrameRate();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Control Panel");
	ImGui::Text(ss.str().c_str());
	ImGui::SliderFloat("Fog Density", &sc.FOG_DENSITY, 0.0f, 0.008f);
	ImGui::SliderFloat("Light Intensity", &sc.LIGHT_I, 0.0f, 60.0f);
	ImGui::SliderFloat("Ambient Intensity", &sc.AMBIENT_I, 0.0f, 0.2f);
	ImGui::End();
}


void drawLoop() {
	prepareImguiFrame();
	static int cur = 0;
	uint32_t imgIdx;
	st.fence->waitAndReset();

	updateUniform();
	st.rd.drPrepareFrame(cur, &imgIdx);

	// ImGUI draw calls
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	st.rd.exRenderImGui(cur, st.fxaaPass->getSwapchainFb(), {0,0,0,1}, drawData);
	uint32_t imguiCmdBuf;
	st.rd.exGetImGuiCommandBufferIndex(cur, &imguiCmdBuf);
	AnthemSemaphore* imguiSemaphore;
	st.rd.exGetImGuiDrawProgressSemaphore(cur, &imguiSemaphore);

	// Main draw calls
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.deferPass->getCommandIndex(cur), -1, {}, {}, nullptr, { st.deferComplete });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.shadowPass->getCommandIndex(cur), -1, { st.deferComplete }, { AT_SSW_ALL_COMMAND }, nullptr, { st.shadowComplete });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.aoPass->getCommandIndex(cur), -1, { st.shadowComplete }, { AT_SSW_ALL_COMMAND }, nullptr, { st.aoComplete });
	st.rd.drSubmitCommandBufferCompQueueGeneralA(st.lightPartPass->getCommandIndex(cur), { st.aoComplete }, { st.lightScatAttrComplete }, nullptr);
	st.rd.drSubmitCommandBufferCompQueueGeneralA(st.rayMarchPass->getCommandIndex(cur), { st.lightScatAttrComplete }, { st.rayMarchComplete }, nullptr);
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.mainPass->getCommandIndex(cur), -1, { st.rayMarchComplete }, { AT_SSW_ALL_COMMAND }, nullptr, { st.mainComplete });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.fxaaPass->getCommandIdx(cur), cur, { st.mainComplete }, { AT_SSW_ALL_COMMAND }, st.fence, { st.aaComplete });
	st.rd.drSubmitCommandBufferGraphicsQueueGeneralA(imguiCmdBuf, cur, { st.aaComplete }, { AT_SSW_ALL_COMMAND });
	st.rd.drPresentFrame(cur, imgIdx);
	cur = (cur + 1) % 2;
}


int main() {
	initialize();
	setupImgui();
	loadModel();
	prepareAOSamples();
	prepareCanvas();
	prepareShadow();
	prepareDefer();
	prepareFogVolume();
	prepareLightParticipationPass();
	prepareRayMarchingPass();
	prepareMain();
	preparePostAA();
	prepareIdPass();
	st.rd.registerPipelineSubComponents();
	recordCommand();
	st.rd.setDrawFunction(drawLoop);
	st.rd.startDrawLoopDemo();
	st.rd.exDestroyImgui();
	st.rd.finalize();
	return 0;
}