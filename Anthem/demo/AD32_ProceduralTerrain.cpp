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
#include "../include/components/passhelper/AnthemSequentialCommand.h"
#include "../include/components/postprocessing/AnthemPostIdentity.h"
#include "../include/components/math/AnthemLowDiscrepancySequence.h"
#include "../include/components/math/AnthemNoise.h"
#include "../include/components/postprocessing/AnthemGlobalFog.h"
#include "../include/components/postprocessing/AnthemFXAA.h"
#include "../include/components/postprocessing/AnthemSimpleBlur.h"
#include "../include/components/camera/AnthemOrbitControl.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::Components::PassHelper;
using namespace Anthem::Components::Postprocessing;
using namespace Anthem::Components::Math;
using namespace Anthem::External;
using namespace Anthem::Core;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "procterrain\\pt.";
	st += x;
	st += ".hlsl.spv";
	return st;
}
#define DCONST static constexpr const
struct StageConstants {
	DCONST int32_t CHUNK_SIZE_X = 168;
	DCONST int32_t CHUNK_SIZE_ELEVATION = 168;

	DCONST int32_t COMPUTE_GROUP_X = 8;
	DCONST int32_t COMPUTE_GROUP_Y = 8;
	DCONST int32_t COMPUTE_GROUP_Z = 8;
	DCONST int32_t DEMO_X = 1;
	DCONST int32_t DEMO_Z = 1;

	DCONST int32_t MAX_TRIANGLES = 190000*15;
	DCONST float WATER_WIDTH = 1600;
	DCONST float WATER_ELEVATION = 50;
	DCONST float SKYBOX_SIZE = 800;
	DCONST float SKYBOX_ELEVATION = 50;
}sc;
#undef DCONST



struct Chunk {
	int32_t x, z;
	AnthemImage* volDensity;
	AnthemShaderStorageBufferImpl<AtBufVecd4f<1>, AtBufVecd4f<1>, AtBufVecd4f<1>>* mesh;
	AnthemPushConstantImpl<AtBufVec4f<1>>* pc; //(Loclx,Loclz,0,0)
	AnthemDescriptorPool* descVol;
	AnthemDescriptorPool* descMesh;

	std::vector<AnthemDescriptorSetEntry> dseVolPass;
	std::vector<AnthemDescriptorSetEntry> dseMarchingCubePass;
	uint32_t cmdVolPass, cmdMcPass, cmdDrawPass;

	AnthemSemaphore* volPassDone;
	AnthemFence* mcPassDone;
	AnthemIndexBuffer* ix;
	int gpuTriangles = 0;
};

struct Surface {
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>, AtAttributeVecf<4>>* vx;
	AnthemIndexBuffer* ix;
}sf;


struct Stage {
	// Base
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemOrbitControl orbit;
	AnthemCamera cam = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemDescriptorPool* descCamera;
	AnthemFrameRateMeter frm = AnthemFrameRateMeter(10);
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>, AtUniformVecf<4>>* ubCamera;

	// Geometry
	std::map<std::pair<int, int>, Chunk> chunks;
	Chunk* demoChunk;

	// Render Target (G-Buffers)
	AnthemDescriptorPool* descGColor;
	AnthemDescriptorPool* descGNormal;
	AnthemDescriptorPool* descGPos;
	AnthemDescriptorPool* descGAO;
	AnthemDescriptorPool* descGTangent;
	AnthemDescriptorPool* descGDeferBlend;
	AnthemDescriptorPool* descGUnderwaterMask;

	AnthemImage* gColor;
	AnthemImage* gNormal;
	AnthemImage* gPos;
	AnthemImage* gAO;
	AnthemImage* gTangent;
	AnthemImage* gDeferBlend;
	AnthemImage* gUnderWaterMask;

	// Passes
	std::unique_ptr<AnthemComputePassHelper> passVol;
	std::unique_ptr<AnthemComputePassHelper> passMarchingCube;
	std::unique_ptr<AnthemPassHelper> passDefer;
	std::unique_ptr<AnthemPassHelper> passAO;
	std::unique_ptr<AnthemSimpleBlur> passAOPost;
	std::unique_ptr<AnthemPassHelper> passDeferBlend;
	std::unique_ptr<AnthemPassHelper> passSurface;
	std::unique_ptr<AnthemPassHelper> passSkybox;
	std::unique_ptr<AnthemFXAA> passFXAA;

	// Command Sequence
	std::function<void(int, int, int, int)> keyController;
	std::unique_ptr<AnthemSequentialCommand> seq[2];

	// Counters
	int gpuTrianlges = 0;

	// Ambient Occlusion
	using uxSamples = AtUniformVecfArray<4, 64>;
	AnthemDescriptorPool* descAOSamples;
	AnthemUniformBufferImpl<uxSamples>* aoSamples;

	// Skybox
	AnthemVertexBufferImpl<AtAttributeVecf<4>>* sbox;
	AnthemIndexBuffer* ixBox;
	AnthemDescriptorPool* descBox;
	AnthemImageCubic* texSkybox;
}st;

struct DeferMesh {
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>>* vx;
	AnthemIndexBuffer* ix;
}canvas;

void initialize() {
	st.cfg.demoName = "32. Procedural Terrain";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.cam.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.01f, 10000.0f, 1.0f * rdW / rdH);
	st.cam.specifyPosition(4, 204, -284);
	st.cam.specifyFrontEyeRay(0, 0, 1);

	st.keyController = st.cam.getKeyboardController(2.4);
	st.rd.ctSetKeyBoardController(st.keyController);
}

void createChunk(int x, int z) {
	st.chunks[{x, z}] = {};
	auto& c = st.chunks[{x, z}];
	c.x = x;
	c.z = z;
	st.rd.createDescriptorPool(&c.descVol);
	st.rd.createTexture3d(&c.volDensity, c.descVol, nullptr, sc.CHUNK_SIZE_X + 1, sc.CHUNK_SIZE_ELEVATION + 1, sc.CHUNK_SIZE_X + 1,
		1, 0, AT_IF_SIGNED_FLOAT32_MONO, -1, AT_IU_COMPUTE_OUTPUT);
	c.volDensity->toGeneralLayout();
	st.rd.addStorageImageArrayToDescriptor({ c.volDensity }, c.descVol, 0, -1);
	st.rd.createDescriptorPool(&c.descMesh);
	uint32_t chunkSize = std::min(static_cast<uint32_t>(sc.MAX_TRIANGLES), sc.CHUNK_SIZE_X * sc.CHUNK_SIZE_ELEVATION * sc.CHUNK_SIZE_X * 15u);
	using ssboType = std::remove_cv_t<decltype(c.mesh)>;
	std::optional<std::function<void(ssboType)>> initFunc = std::nullopt;
	st.rd.createShaderStorageBuffer(&c.mesh, chunkSize, 0, c.descMesh, initFunc, -1);

	c.dseVolPass = {
		{c.descVol,AT_ACDS_STORAGE_IMAGE,0}
	};
	c.dseMarchingCubePass = {
		{c.descVol,AT_ACDS_STORAGE_IMAGE,0},
		{c.descMesh,AT_ACDS_SHADER_STORAGE_BUFFER,0}
	};
	st.rd.createPushConstant(&c.pc);
	c.pc->enableShaderStage(AT_APCS_COMPUTE);
	c.pc->enableShaderStage(AT_APCS_VERTEX);
	c.pc->enableShaderStage(AT_APCS_FRAGMENT);
	auto sA = 0;
	auto sB = 0;
	float constpc[4] = { (float)x,(float)z,sA,sB };

	ANTH_LOGI("Random number is:", sA, "-", sB);

	c.pc->setConstant(constpc);

	if (x == sc.DEMO_X && z == sc.DEMO_Z) {
		st.demoChunk = &c;
	}
}

void createGeometrySkybox() {
	auto c = sc.SKYBOX_SIZE;
	auto d = sc.SKYBOX_ELEVATION;
	const std::vector<std::array<float, 4>> vertices = {
		{-c,-c,-c,c},{c,-c,-c,c},{c,c,-c,c},{-c,c,-c,c},
		{-c,-c,c,c},{c,-c,c,c},{c,c,c,c},{-c,c,c,c}
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
	st.rd.createIndexBuffer(&st.ixBox);
	st.ixBox->setIndices(indices);
	st.sbox->setTotalVertices(8);
	for (auto i : AT_RANGE(0, 8))st.sbox->insertData(i, vertices[i]);
}

inline std::string getSkyboxTex(auto x) {
	std::string st(ANTH_ASSET_DIR);
	st += "skybox2\\";
	st += x;
	st += ".png";
	return st;
}


void createTextures() {
	const char* fileNames[6] = { "px","nx","py","ny","pz","nz" };
	std::array<uint8_t*, 6> rawData;
	uint32_t width, height, channel;
	AnthemImageLoader loader;
	for (auto i : AT_RANGE(0, 6)) {
		loader.loadImage(getSkyboxTex(fileNames[i]).c_str(), &width, &height, &channel, &rawData[i]);
	}
	st.rd.createDescriptorPool(&st.descBox);
	st.rd.createCubicTextureSimple(&st.texSkybox, st.descBox, rawData, width, height, channel, 0, -1);
}

void recordChunkCommands(int x, int z) {
	auto& c = st.chunks[{x, z}];
	st.rd.drAllocateCommandBuffer(&c.cmdVolPass);
	st.rd.drAllocateCommandBuffer(&c.cmdMcPass);

	// Vol Pass
	st.rd.drStartCommandRecording(c.cmdVolPass);
	st.rd.drBindComputePipeline(st.passVol->pipeline, c.cmdVolPass);
	st.rd.drBindDescriptorSetCustomizedCompute(c.dseVolPass, st.passVol->pipeline, c.cmdVolPass);
	int gx = sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_X;
	int gy = sc.CHUNK_SIZE_ELEVATION / sc.COMPUTE_GROUP_Y;
	int gz = sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_Z;
	st.rd.drPushConstantsCompute(c.pc, st.passVol->pipeline, c.cmdVolPass);
	st.rd.drComputeDispatch(c.cmdVolPass, gx, gy, gz);
	st.rd.drEndCommandRecording(c.cmdVolPass);

	// Marching Cube Pass
	st.rd.drStartCommandRecording(c.cmdMcPass);
	st.rd.drBindComputePipeline(st.passMarchingCube->pipeline, c.cmdMcPass);
	st.rd.drBindDescriptorSetCustomizedCompute(c.dseMarchingCubePass, st.passMarchingCube->pipeline, c.cmdMcPass);
	st.rd.drPushConstantsCompute(c.pc, st.passMarchingCube->pipeline, c.cmdMcPass);
	st.rd.drComputeDispatch(c.cmdMcPass, gx, gy, gz);
	st.rd.drEndCommandRecording(c.cmdMcPass);

	// Submit & Wait
	st.rd.createSemaphore(&c.volPassDone);
	st.rd.createFence(&c.mcPassDone);
	c.mcPassDone->resetFence();
	st.rd.drSubmitCommandBufferCompQueueGeneralA(c.cmdVolPass, {}, { c.volPassDone },nullptr);
	st.rd.drSubmitCommandBufferCompQueueGeneralA(c.cmdMcPass, { c.volPassDone }, {}, c.mcPassDone);
	c.mcPassDone->waitAndReset();
}

void createComputePass() {
	// Vol Pass : Generate Volume Density
	st.passVol = std::make_unique<AnthemComputePassHelper>(&st.rd, 1);
	st.passVol->workGroupSize = { sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_X, sc.CHUNK_SIZE_ELEVATION / sc.COMPUTE_GROUP_Y, sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_Z };
	st.passVol->shaderPath.computeShader = getShader("vol.comp");
	st.passVol->pushConstants = {st.demoChunk->pc};
	st.passVol->setDescriptorLayouts(st.demoChunk->dseVolPass);
	st.passVol->buildComputePipeline();

	// Marching Cube Pass : Generate Mesh
	st.passMarchingCube = std::make_unique<AnthemComputePassHelper>(&st.rd, 1);
	st.passMarchingCube->workGroupSize = { sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_X, sc.CHUNK_SIZE_ELEVATION / sc.COMPUTE_GROUP_Y, sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_Z };
	st.passMarchingCube->shaderPath.computeShader = getShader("mc.comp");
	st.passMarchingCube->pushConstants = { st.demoChunk->pc };
	st.passMarchingCube->setDescriptorLayouts(st.demoChunk->dseMarchingCubePass);
	st.passMarchingCube->buildComputePipeline();
}

void createSkyPass() {
	st.passSkybox = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.passSkybox->shaderPath.vertexShader = getShader("sky.vert");
	st.passSkybox->shaderPath.fragmentShader = getShader("sky.frag");
	st.passSkybox->vxLayout = st.sbox;
	st.passSkybox->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passSkybox->setRenderTargets({ st.gColor, st.gNormal,st.gPos, st.gTangent });
	st.passSkybox->passOpt.clearColors = { {0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f,0.0f} };
	st.passSkybox->pipeOpt.blendPreset = { AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
	st.passSkybox->passOpt.clearColorAttachmentOnLoad = { false,false,false,false };
	st.passSkybox->passOpt.clearDepthAttachmentOnLoad = false;
	st.passSkybox->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32 };
	st.passSkybox->pipeOpt.blendPreset = { AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
	st.passSkybox->setDescriptorLayouts({
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descBox,AT_ACDS_SAMPLER,0},
	});
	st.passSkybox->setDepthFromPass(*st.passDefer);
	st.passSkybox->buildGraphicsPipeline();
}


void createGraphicsPass() {
	// Defer Pass
	st.passDefer = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passDefer->shaderPath.fragmentShader = getShader("draw.frag");
	st.passDefer->shaderPath.vertexShader = getShader("draw.vert");
	st.passDefer->pipeOpt.enableCullMode = true;
	st.passDefer->pipeOpt.cullMode = AT_ACM_BACK;
	st.passDefer->pipeOpt.frontFace = AT_AFF_COUNTER_CLOCKWISE;
	st.passDefer->setRenderTargets({ st.gColor, st.gNormal,st.gPos, st.gTangent });
	st.passDefer->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32 };
	st.passDefer->passOpt.clearColors={{0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f,0.0f} };
	st.passDefer->pipeOpt.blendPreset = { AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
	st.passDefer->setDescriptorLayouts({
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.passDefer->vxLayout = st.demoChunk->mesh;
	st.passDefer->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passDefer->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32 };
	st.passDefer->passOpt.clearColorAttachmentOnLoad = { true,true,true,true };
	st.passDefer->buildGraphicsPipeline();

	// AO Pass
	st.passAO = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passAO->shaderPath.fragmentShader = getShader("ao.frag");
	st.passAO->shaderPath.vertexShader = getShader("ao.vert");
	st.passAO->setRenderTargets({st.gAO });
	st.passAO->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.passAO->setDescriptorLayouts({
		{st.descGColor,AT_ACDS_SAMPLER,0},
		{st.descGNormal,AT_ACDS_SAMPLER,0},
		{st.descGPos,AT_ACDS_SAMPLER,0},
		{st.descGTangent,AT_ACDS_SAMPLER,0},
		{st.descAOSamples,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.passAO->vxLayout = canvas.vx;
	st.passAO->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passAO->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.passAO->buildGraphicsPipeline();

	// Water Pass I: Get Mask for Underwater stuffs
	st.passSurface = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passSurface->shaderPath.fragmentShader = getShader("water.frag");
	st.passSurface->shaderPath.vertexShader = getShader("water.vert");
	st.passSurface->setRenderTargets({ st.gUnderWaterMask });
	st.passSurface->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.passSurface->passOpt.storeDepthValues = true;
	st.passSurface->pipeOpt.enableDepthWriting = false;
	st.passSurface->passOpt.clearDepthAttachmentOnLoad = false;
	st.passSurface->passOpt.clearColorAttachmentOnLoad = { true };
	st.passSurface->setDepthFromPass(*st.passDefer);
	st.passSurface->setDescriptorLayouts({
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0}
		});
	st.passSurface->vxLayout = st.demoChunk->mesh;
	st.passSurface->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passSurface->buildGraphicsPipeline();

	// AO Post
	st.passAOPost = std::make_unique<AnthemSimpleBlur>(&st.rd, 2);
	st.passAOPost->addInput({
		{st.descGAO,AT_ACDS_SAMPLER,0},
	});
	st.passAOPost->prepare(true);

	// Defer Blend
	st.passDeferBlend = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passDeferBlend->shaderPath.fragmentShader = getShader("blend.frag");
	st.passDeferBlend->shaderPath.vertexShader = getShader("blend.vert");
	st.passDeferBlend->setRenderTargets({ st.gDeferBlend });
	st.passDeferBlend->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.passDeferBlend->setDescriptorLayouts({
		{st.descGColor,AT_ACDS_SAMPLER,0},
		{st.descGNormal,AT_ACDS_SAMPLER,0},
		{st.descGPos,AT_ACDS_SAMPLER,0},
		{st.passAOPost->getColorAttachmentDescId(0),AT_ACDS_SAMPLER,0},
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descGUnderwaterMask,AT_ACDS_SAMPLER,0}
	});

	st.passDeferBlend->vxLayout = canvas.vx;
	st.passDeferBlend->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passDeferBlend->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.passDeferBlend->buildGraphicsPipeline();

	// FXAA Pass
	st.passFXAA = std::make_unique<AnthemFXAA>(&st.rd, 2);
	st.passFXAA->addInput({
		{st.descGDeferBlend,AT_ACDS_SAMPLER,0},
		});
	st.passFXAA->prepare(false);
}

void prepareAOSamples() {
	std::vector<float> samples(4 * 64);
	for (int i = 0; i < 4 * 64; i += 4) {
		const auto d = AnthemLinAlg::randomVector3<float>();
		samples[i] = d[0];
		samples[i + 1] = d[1];
		samples[i + 2] = d[2];
		samples[i + 3] = (rand() % 10);
	}
	st.aoSamples->specifyUniforms(samples.data());
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.aoSamples->updateBuffer(i);
	}
}


void createUniform() {
	st.rd.createDescriptorPool(&st.descCamera);
	st.rd.createUniformBuffer(&st.ubCamera, 0, st.descCamera, -1);

	// GBuffers
	st.rd.createDescriptorPool(&st.descGColor);
	st.rd.createColorAttachmentImage(&st.gColor, st.descGColor, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descGNormal);
	st.rd.createColorAttachmentImage(&st.gNormal, st.descGNormal, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descGPos);
	st.rd.createColorAttachmentImage(&st.gPos, st.descGPos, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descGAO);
	st.rd.createColorAttachmentImage(&st.gAO, st.descGAO, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descAOSamples);
	st.rd.createUniformBuffer(&st.aoSamples, 0, st.descAOSamples, -1);
	st.rd.createDescriptorPool(&st.descGTangent);
	st.rd.createColorAttachmentImage(&st.gTangent, st.descGTangent, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descGDeferBlend);
	st.rd.createColorAttachmentImage(&st.gDeferBlend, st.descGDeferBlend, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descGUnderwaterMask);
	st.rd.createColorAttachmentImage(&st.gUnderWaterMask, st.descGUnderwaterMask, 0, AT_IF_SIGNED_FLOAT32, false, -1);
}

void createWaterGeometry() {
	st.rd.createVertexBuffer(&sf.vx);
	st.rd.createIndexBuffer(&sf.ix);

	sf.vx->setTotalVertices(4);
	sf.vx->insertData(0, { -sc.WATER_WIDTH,sc.WATER_ELEVATION,-sc.WATER_WIDTH,1 }, { 0,1,0,0 }, { 1,0,0,0 });
	sf.vx->insertData(1, { sc.WATER_WIDTH,sc.WATER_ELEVATION,-sc.WATER_WIDTH,1 }, { 1,1,0,0 }, { 1,0,0,0 });
	sf.vx->insertData(2, { sc.WATER_WIDTH,sc.WATER_ELEVATION,sc.WATER_WIDTH,1 }, { 1,0,0,0 }, { 1,0,0,0 });
	sf.vx->insertData(3, { -sc.WATER_WIDTH,sc.WATER_ELEVATION,sc.WATER_WIDTH,1 }, { 0,0,0,0 }, { 1,0,0,0 });

	sf.ix->setIndices({ 0,1,2,2,3,0 });
}

void createIndexBuffer(int x,int z) {
	st.rd.createIndexBuffer(&st.chunks[{x, z}].ix);
	std::vector<uint32_t> indices = {};
	st.chunks[{x, z}].gpuTriangles = st.chunks[{x, z}].mesh->getAtomicCounter(0);
	ANTH_LOGI("Valid idx",st.chunks[{x, z}].gpuTriangles);
	auto lm = st.chunks[{x, z}].gpuTriangles * 3 + 3;
	for (auto i : AT_RANGE2(lm)) {
		indices.push_back(i);
	}
	st.chunks[{x, z}].ix->setIndices(indices);
	st.chunks[{x, z}].ix->createBuffer();
}

void prepareDeferCanvas() {
	st.rd.createVertexBuffer(&canvas.vx);
	st.rd.createIndexBuffer(&canvas.ix);
	canvas.vx->setTotalVertices(4);
	float dw = 1.0;
	canvas.vx->insertData(0, { -dw,-dw,0,1 }, { 0,0,0,0 });
	canvas.vx->insertData(1, { dw,-dw,0,1 }, { 1,0,0,0 });
	canvas.vx->insertData(2, { dw,dw,0,1 }, { 1,1,0,0 });
	canvas.vx->insertData(3, { -dw,dw,0,1 }, { 0,1,0,0 });
	canvas.ix->setIndices({ 0,1,2,2,3,0 });
}

void injectCommands(uint32_t x) {
	ANTH_LOGI(st.chunks.size(), "SIZE");
	for (auto& [k, v] : st.chunks) {
		ANTH_LOGI("Add commands", k.first, " ", k.second);
		st.rd.drBindVertexBufferFromSsbo(v.mesh, 0, x);
		st.rd.drBindIndexBuffer(v.ix, x);
		st.rd.drDraw(v.ix->getIndexCount(), x);
	}
}

void recordDrawCommand() {
	st.passSurface->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(sf.vx, x);
		st.rd.drBindIndexBuffer(sf.ix, x);
		st.rd.drDraw(sf.ix->getIndexCount(), x);
	});
	st.passDefer->recordCommands([&](uint32_t x) {
		injectCommands(x);
	});
	st.passAO->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(canvas.vx, x);
		st.rd.drBindIndexBuffer(canvas.ix, x);
		st.rd.drDraw(6, x);
	});
	st.passDeferBlend->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(canvas.vx, x);
		st.rd.drBindIndexBuffer(canvas.ix, x);
		st.rd.drDraw(6, x);
	});
	st.passSkybox->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.sbox, x);
		st.rd.drBindIndexBuffer(st.ixBox, x);
		st.rd.drDraw(st.ixBox->getIndexCount(), x);
	});
	st.passAOPost->recordCommandOffscreen();
	st.passFXAA->recordCommand();

	st.seq[0] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.seq[1] = std::make_unique<AnthemSequentialCommand>(&st.rd);

	st.seq[0]->setSequence({
		{ st.passDefer->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{st.passSurface->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{st.passSkybox->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{ st.passAO->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.passAOPost->getCommandIdx(0),ATC_ASCE_GRAPHICS},
		{ st.passDeferBlend->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{ st.passFXAA->getCommandIdx(0), ATC_ASCE_GRAPHICS},
	});
	st.seq[1]->setSequence({
		{ st.passDefer->getCommandIndex(1), ATC_ASCE_GRAPHICS} ,
		{st.passSurface->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{st.passSkybox->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{ st.passAO->getCommandIndex(1), ATC_ASCE_GRAPHICS},
		{ st.passAOPost->getCommandIdx(1),ATC_ASCE_GRAPHICS},
		{ st.passDeferBlend->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{ st.passFXAA->getCommandIdx(1), ATC_ASCE_GRAPHICS},
	});
}

void updateUniform() {
	AtMatf4 proj, view, local;
	AtVecf3 camPos;
	st.cam.getProjectionMatrix(proj);
	st.cam.getViewMatrix(view);
	st.cam.getPosition(camPos);
	local = AnthemLinAlg::eye<float, 4>();

	float pm[16], vm[16], lm[16], vc[4];

	vc[0] = camPos[0];
	vc[1] = camPos[1];
	vc[2] = camPos[2];
	vc[3] = 0;
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);
	st.ubCamera->specifyUniforms(pm, vm, lm,vc);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.ubCamera->updateBuffer(i);
	}
}

void createAllChunks() {
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			createChunk(i, j);
		}
	}
}

void recordAllChunks() {
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			recordChunkCommands(i, j);
		}
	}
}
void createAllIndexBuffer() {
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			createIndexBuffer(i, j);
		}
	}

}


void drawLoop() {
	static int cur = 0;
	updateUniform();
	uint32_t imgIdx = 0;
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.seq[cur]->executeCommandToStage(imgIdx, false, false, st.passFXAA->getSwapchainFb());
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}

int main() {
	initialize();
	createUniform();
	createTextures();
	createWaterGeometry();
	createGeometrySkybox();
	prepareAOSamples();
	prepareDeferCanvas();
	createAllChunks();
	createComputePass();
	createGraphicsPass();
	createSkyPass();

	st.rd.registerPipelineSubComponents();
	recordAllChunks();
	ANTH_LOGI("Chunk created");

	createAllIndexBuffer();
	recordDrawCommand();

	st.rd.setDrawFunction(drawLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();

	return 0;
}