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
	DCONST int32_t CHUNK_SIZE_X = 256;
	DCONST int32_t CHUNK_SIZE_ELEVATION = 256;

	DCONST int32_t COMPUTE_GROUP_X = 8;
	DCONST int32_t COMPUTE_GROUP_Y = 8;
	DCONST int32_t COMPUTE_GROUP_Z = 8;
	DCONST int32_t DEMO_X = 0;
	DCONST int32_t DEMO_Z = 0;

	DCONST int32_t MAX_TRIANGLES = 600000;
}sc;
#undef DCONST
 
struct Chunk {
	int32_t x, z;
	AnthemImage* volDensity;
	AnthemShaderStorageBufferImpl<AtBufVecd4f<1>, AtBufVecd4f<1>>* mesh;
	AnthemPushConstantImpl<AtBufVec4f<1>>* pc; //(Loclx,Loclz,0,0)
	AnthemDescriptorPool* descVol;
	AnthemDescriptorPool* descMesh;

	std::vector<AnthemDescriptorSetEntry> dseVolPass;
	std::vector<AnthemDescriptorSetEntry> dseMarchingCubePass;
	uint32_t cmdVolPass, cmdMcPass, cmdDrawPass;

	AnthemSemaphore* volPassDone;
	AnthemFence* mcPassDone;
};

struct Stage {
	// Base
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemOrbitControl orbit;
	AnthemCamera cam = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemDescriptorPool* descCamera;
	AnthemFrameRateMeter frm = AnthemFrameRateMeter(10);
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* ubCamera;

	// Geometry
	AnthemIndexBuffer* index;
	std::map<std::pair<int, int>, Chunk> chunks;
	Chunk* demoChunk;

	// Render Target
	AnthemDescriptorPool* descTarget;
	AnthemImage* canvas;

	// Passes
	std::unique_ptr<AnthemComputePassHelper> passVol;
	std::unique_ptr<AnthemComputePassHelper> passMarchingCube;
	std::unique_ptr<AnthemPassHelper> passDraw;
	std::unique_ptr<AnthemFXAA> passFXAA;

	// Command Sequence
	std::function<void(int, int, int, int)> keyController;
	std::unique_ptr<AnthemSequentialCommand> seq[2];

	// Counters
	int gpuTrianlges = 0;
}st;

void initialize() {
	st.cfg.demoName = "32. Procedural Terrain";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.cam.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.01f, 1000.0f, 1.0f * rdW / rdH);
	st.cam.specifyPosition(0, 8, -2);
	st.cam.specifyFrontEyeRay(0, 0, 1);

	st.keyController = st.cam.getKeyboardController(0.1);
	st.rd.ctSetKeyBoardController(st.keyController);
}

void createChunk(int x, int z) {
	st.chunks[{x, z}] = {};
	auto& c = st.chunks[{x, z}];
	c.x = x;
	c.z = z;
	st.rd.createDescriptorPool(&c.descVol);
	st.rd.createTexture3d(&c.volDensity, c.descVol, nullptr, sc.CHUNK_SIZE_X+1, sc.CHUNK_SIZE_ELEVATION+1, sc.CHUNK_SIZE_X+1,
		1, 0, AT_IF_SIGNED_FLOAT32_MONO, -1,AT_IU_COMPUTE_OUTPUT);
	c.volDensity->toGeneralLayout();
	st.rd.addStorageImageArrayToDescriptor({ c.volDensity }, c.descVol, 0, -1);
	st.rd.createDescriptorPool(&c.descMesh);
	uint32_t chunkSize = std::min(sc.MAX_TRIANGLES, sc.CHUNK_SIZE_X * sc.CHUNK_SIZE_ELEVATION * sc.CHUNK_SIZE_X * 15);
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
	float constpc[4] = { (float)x,(float)z,0,0 };
	c.pc->setConstant(constpc);

	if (x == sc.DEMO_X && z == sc.DEMO_Z) {
		st.demoChunk = &c;
	}
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

void createGraphicsPass() {
	st.passDraw = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passDraw->shaderPath.fragmentShader = getShader("draw.frag");
	st.passDraw->shaderPath.vertexShader = getShader("draw.vert");
	st.passDraw->setRenderTargets({ st.canvas });
	st.passDraw->setDescriptorLayouts({
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.passDraw->vxLayout = st.demoChunk->mesh;
	st.passDraw->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passDraw->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.passDraw->buildGraphicsPipeline();

	st.passFXAA = std::make_unique<AnthemFXAA>(&st.rd, 2);
	st.passFXAA->addInput({
		{st.descTarget,AT_ACDS_SAMPLER,0},
		});
	st.passFXAA->prepare(false);
}

void createUniform() {
	st.rd.createDescriptorPool(&st.descCamera);
	st.rd.createUniformBuffer(&st.ubCamera, 0, st.descCamera, -1);
	st.rd.createDescriptorPool(&st.descTarget);
	st.rd.createColorAttachmentImage(&st.canvas, st.descTarget, 0, AT_IF_SIGNED_FLOAT32, false, -1);
}

void createIndexBuffer() {
	st.gpuTrianlges = 1;
	for(auto& [k,v]:st.chunks) {
		st.gpuTrianlges += v.mesh->getAtomicCounter(0);
	}
	st.rd.createIndexBuffer(&st.index);
	std::vector<uint32_t> idx;
	for (auto i : AT_RANGE2(st.gpuTrianlges * 3)) {
		idx.push_back(i);
	}
	st.index->setIndices(idx);
	st.index->createBuffer();
	ANTH_LOGI("Total Triangles = ", st.gpuTrianlges, " Total Indicecs = ", idx.size());
}

void recordDrawCommand() {
	st.passDraw->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBufferFromSsbo(st.demoChunk->mesh,0, x);
		st.rd.drBindIndexBuffer(st.index, x);
		st.rd.drDraw(st.index->getIndexCount(), x);
	});
	st.passFXAA->recordCommand();

	st.seq[0] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.seq[1] = std::make_unique<AnthemSequentialCommand>(&st.rd);

	st.seq[0]->setSequence({
		{ st.passDraw->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.passFXAA->getCommandIdx(0), ATC_ASCE_GRAPHICS},
		});
	st.seq[1]->setSequence({
		{ st.passDraw->getCommandIndex(1), ATC_ASCE_GRAPHICS} ,
		{ st.passFXAA->getCommandIdx(1), ATC_ASCE_GRAPHICS},
	});
}

void updateUniform() {
	AtMatf4 proj, view, local;
	st.cam.getProjectionMatrix(proj);
	st.cam.getViewMatrix(view);
	local = AnthemLinAlg::eye<float, 4>();

	float pm[16], vm[16], lm[16], vc[4];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);
	st.ubCamera->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.ubCamera->updateBuffer(i);
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

	createChunk(sc.DEMO_X, sc.DEMO_Z);
	createComputePass();
	createGraphicsPass();

	st.rd.registerPipelineSubComponents();
	recordChunkCommands(sc.DEMO_X, sc.DEMO_Z);
	ANTH_LOGI("Chunk created");

	createIndexBuffer();
	recordDrawCommand();

	st.rd.setDrawFunction(drawLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();

	return 0;
}