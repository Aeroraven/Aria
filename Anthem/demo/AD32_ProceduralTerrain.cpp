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
	DCONST int32_t CHUNK_SIZE_X = 96;
	DCONST int32_t CHUNK_SIZE_ELEVATION = 144;

	DCONST int32_t COMPUTE_GROUP_X = 8;
	DCONST int32_t COMPUTE_GROUP_Y = 8;
	DCONST int32_t COMPUTE_GROUP_Z = 8;
	DCONST int32_t COMPUTE_GROUP_CULLING = 64;
	DCONST int32_t DEMO_X = 1;
	DCONST int32_t DEMO_Z = 1;

	DCONST int32_t MAX_TRIANGLES = 270000;
	DCONST float WATER_WIDTH = 1600;
	DCONST float WATER_ELEVATION = 50;
	DCONST float SKYBOX_SIZE = 15800;
	DCONST float SKYBOX_ELEVATION = 50;
	DCONST float WORLD_SIZE = 3;
	DCONST float WORLD_SIZEZ = 4;
	DCONST float WORLD_SIZENZ = -2;
	DCONST int32_t MAX_TREES = 2000;

	DCONST int32_t CSM_LEVELS = 7;
	DCONST std::array<float, CSM_LEVELS + 1> CSM_RANGES = { 0.1f, 200.0f, 400.0f, 600.0f,800.0f, 1500.0f,4000.0f,20000.0f };
	DCONST int32_t CSM_RESOL = 4200;

	float FOG_SCATTER = 0.0005;
	float FOG_HEIGHT_ATTENUATION = 0.004;
	float FOG_ELEVATION = 0;
}sc;
#undef DCONST



struct Chunk {
	int32_t x, z;
	int32_t lod;
	AnthemImage* volDensity;
	AnthemShaderStorageBufferImpl<AtBufVecd4f<1>, AtBufVecd4f<1>, AtBufVecd4f<1>>* mesh;
	AnthemShaderStorageBufferImpl<AtBufVecd4f<1>>* treeInstancing;

	AnthemPushConstantImpl<AtBufVec4f<1>>* pc; //(Loclx,Loclz,Lod,0)
	AnthemDescriptorPool* descVol;
	AnthemDescriptorPool* descMesh;
	AnthemDescriptorPool* descTreeInst;

	std::vector<AnthemDescriptorSetEntry> dseVolPass;
	std::vector<AnthemDescriptorSetEntry> dseMarchingCubePass;
	uint32_t cmdVolPass, cmdMcPass, cmdDrawPass, cmdCullpass;

	AnthemSemaphore* volPassDone;
	AnthemSemaphore* cullSemaphore;
	AnthemFence* mcPassDone;
	AnthemIndexBuffer* ix;
	int gpuTriangles = 0;
};

struct Surface {
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>, AtAttributeVecf<4>, AtAttributeVecf<4>>* vx;
	AnthemIndexBuffer* ix;
}sf;


struct Stage {
	// Base
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemOrbitControl orbit;
	AnthemCamera cam = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemCamera* camLight[sc.CSM_LEVELS];

	AnthemDescriptorPool* descCamera;
	AnthemDescriptorPool* descCSMData;
	AnthemDescriptorPool* descGlobalFog;

	AnthemPushConstantImpl<
		AtBufMat4f<1>,
		AtBufMat4f<1>,
		AtBufMat4f<1>,
		AtBufMat4f<1>
	>* pcCSMLight[sc.CSM_LEVELS];

	AnthemFrameRateMeter frm = AnthemFrameRateMeter(10);
	AnthemUniformBufferImpl <
		AtUniformMatf<4>
	>* ubGlobalFog;

	AnthemUniformBufferImpl<
		AtUniformMatf<4>, 
		AtUniformMatf<4>, 
		AtUniformMatf<4>, 
		AtUniformVecf<4>, //Cam Pos
		AtUniformVecf<4>  //Tick
	>* ubCamera;
	AnthemUniformBufferImpl<
		AnthemUniformMatfArray<4,sc.CSM_LEVELS>
	>* ubCameraLight;

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
	AnthemDescriptorPool* descGWaterUV;
	AnthemDescriptorPool* descDebug;

	AnthemImage* gColor;
	AnthemImage* gNormal;
	AnthemImage* gPos;
	AnthemImage* gAO;
	AnthemImage* gTangent;
	AnthemImage* gDeferBlend;
	AnthemImage* gUnderWaterMask;
	AnthemImage* gWaterUV;
	AnthemImage* gDebug;

	AnthemDepthBuffer* gCSMDepth[sc.CSM_LEVELS];
	AnthemDescriptorPool* descCSMDepth;
	AnthemViewport* depthViewport;


	// Water Normals
	AnthemDescriptorPool* descWNormal;
	AnthemImage* waterNormal;
	AnthemDescriptorPool* descWDuDv;
	AnthemImage* waterDuDv;

	// Objects
	std::vector<AnthemGLTFLoaderParseResult> treeGLTF;
	AnthemSimpleModelIntegrator treeModel;
	AnthemGLTFLoaderTexParseResult treeTex;
	AnthemDescriptorPool* descPbrBaseTexTree = nullptr;
	AnthemImage** pbrBaseTexTree = nullptr;
	std::vector<std::string> requiredTexTree;
	AnthemPushConstantImpl<AtBufMat4f<1>>* pcTree;

	// Passes
	std::unique_ptr<AnthemComputePassHelper> passVol[4];
	std::unique_ptr<AnthemComputePassHelper> passMarchingCube[4];

	std::unique_ptr<AnthemPassHelper> passDefer;
	std::unique_ptr<AnthemPassHelper> passDeferTree;

	std::unique_ptr<AnthemPassHelper> passShadow[sc.CSM_LEVELS];
	std::unique_ptr<AnthemPassHelper> passShadowTree[sc.CSM_LEVELS];

	std::unique_ptr<AnthemPassHelper> passAO;
	std::unique_ptr<AnthemSimpleBlur> passAOPost;
	std::unique_ptr<AnthemPassHelper> passDeferBlend;
	std::unique_ptr<AnthemPassHelper> passSurface;
	std::unique_ptr<AnthemPassHelper> passGround;
	std::unique_ptr<AnthemPassHelper> passSkybox;

	std::unique_ptr<AnthemGlobalFog> passFog;
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

	// Sync
	AnthemSemaphore* shadowSemaphore;
	AnthemFence* shadowFence;

	// Compute Fustrum Culling
	std::unique_ptr<AnthemComputePassHelper> passCull;
	AnthemShaderStorageBufferImpl<AtBufVecd4f<1>>* culledTrees;
	AnthemDescriptorPool* descCull;
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
	st.cam.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.01f, 40000.0f, 1.0f * rdW / rdH);
	st.cam.specifyPosition(4,298, -594);
	st.cam.specifyFrontEyeRay(0, 0, 1);

	st.keyController = st.cam.getKeyboardController(2.4);
	st.rd.ctSetKeyBoardController(st.keyController);
}
void initCull() {
	using ssboType = std::remove_cv_t<decltype(st.culledTrees)>;
	std::optional<std::function<void(ssboType)>> initFunc = std::nullopt;
	st.rd.createDescriptorPool(&st.descCull);
	st.rd.createShaderStorageBuffer(&st.culledTrees, sc.MAX_TREES*(sc.WORLD_SIZE*2+1)*(sc.WORLD_SIZEZ+1-sc.WORLD_SIZENZ), 0, st.descCull, initFunc, -1);

}
void initCSM() {
	int rdW, rdH;
	st.rd.exGetWindowSize(rdH, rdW);
	std::vector<float> csmRanges;
	for(auto i:AT_RANGE2(sc.CSM_LEVELS+1))csmRanges.push_back(sc.CSM_RANGES[i]);
	auto aabb = st.cam.getCSMBbox(csmRanges, { 0.0f,0.0f,0.0f });

	std::vector<AnthemImageContainer*> imgContainer;

	float camLightData[16*sc.CSM_LEVELS];
	for (auto i : AT_RANGE2(sc.CSM_LEVELS)) {
		st.rd.createPushConstant(&st.pcCSMLight[i]);
		st.pcCSMLight[i]->enableShaderStage(AT_APCS_VERTEX);
		st.pcCSMLight[i]->enableShaderStage(AT_APCS_FRAGMENT);

		st.camLight[i] = new AnthemCamera(AT_ACPT_ORTHO);
		st.camLight[i]->specifyOrthoClipSpace(0.1f, 40000.0f, 1.0f * rdW / rdH, aabb[i].radius);
		st.camLight[i]->specifyFrontEyeRay(0, -1, -1);
		st.camLight[i]->specifyPosition(aabb[i].center[0], aabb[i].center[1] + 9000, aabb[i].center[2] + 9000);
		//st.camLight[i]->specifyPosition(0, 4000,4000);
		ANTH_LOGI("PPW",aabb[i].center[0], aabb[i].center[1], aabb[i].center[2],aabb[i].radius);

		AtMatf4 proj, view, local;
		st.camLight[i]->getProjectionMatrix(proj);
		st.camLight[i]->getViewMatrix(view);
		local = AnthemLinAlg::eye<float, 4>();
		AtMatf4 pcTreeLocal;
		pcTreeLocal = AnthemLinAlg::axisAngleRotationTransform3<float, float>({ 1.0f,0.0f,0.0f }, AT_PI / 2);

		AtMatf4 mvp = proj.multiply(view);

		float pc[16],vw[16],lc[16],pcTree[16];
		proj.columnMajorVectorization(pc);
		view.columnMajorVectorization(vw);
		local.columnMajorVectorization(lc);
		pcTreeLocal.columnMajorVectorization(pcTree);

		mvp.columnMajorVectorization(camLightData + i * 16);

		st.pcCSMLight[i]->setConstant(pc, vw, lc, pcTree);
	
		st.rd.createDepthBufferWithSampler(&st.gCSMDepth[i], nullptr,0, false,sc.CSM_RESOL,sc.CSM_RESOL);
		imgContainer.push_back(st.gCSMDepth[i]);
	}
	st.rd.createDescriptorPool(&st.descCSMDepth);
	st.rd.addSamplerArrayToDescriptor(imgContainer, st.descCSMDepth, 0, -1);

	st.rd.createDescriptorPool(&st.descCSMData);
	st.rd.createUniformBuffer(&st.ubCameraLight, 0, st.descCSMData, -1);
	st.ubCameraLight->specifyUniforms(camLightData);
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.ubCameraLight->updateBuffer(i);
	}
	st.rd.createViewportCustom(&st.depthViewport, sc.CSM_RESOL, sc.CSM_RESOL, 0, 1);
}

void loadTree() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\tree\\tree.gltf").c_str());
	loader.parseModel(config, st.treeGLTF, &st.treeTex);
	std::vector<AnthemUtlSimpleModelStruct> rp;
	for (auto& p : st.treeGLTF)rp.push_back(p);
	st.treeModel.loadModel(&st.rd, rp, -1);
	st.requiredTexTree = st.treeModel.getRequiredTextures();

	st.rd.createDescriptorPool(&st.descPbrBaseTexTree);

	int numTexs = st.treeTex.tex.size();
	st.pbrBaseTexTree = new AnthemImage * [numTexs];
	std::vector<AnthemImageContainer*> imgContainer;
	for (int i = 0; i < numTexs; i++) {
		st.rd.createTexture(&st.pbrBaseTexTree[i], st.descPbrBaseTexTree, st.treeTex.tex[i].data(), st.treeTex.width[i],
			st.treeTex.height[i], st.treeTex.channels[i], 0, false, false, AT_IF_UNORM_UINT8, -1, true);
		imgContainer.push_back(st.pbrBaseTexTree[i]);
	}
	st.rd.addSamplerArrayToDescriptor(imgContainer, st.descPbrBaseTexTree, 0, -1);

	st.rd.createPushConstant(&st.pcTree);
	st.pcTree->enableShaderStage(AT_APCS_VERTEX);
	st.pcTree->enableShaderStage(AT_APCS_FRAGMENT);

	AtMatf4 pcTreeLocal;
	pcTreeLocal = AnthemLinAlg::axisAngleRotationTransform3<float,float>({1.0f,0.0f,0.0f}, AT_PI / 2);
	float constpc[16];
	pcTreeLocal.columnMajorVectorization(constpc);
	st.pcTree->setConstant(constpc);
}
void prepareGlobalFog() {
	st.rd.createDescriptorPool(&st.descGlobalFog);
	st.rd.createUniformBuffer(&st.ubGlobalFog, 0, st.descGlobalFog, -1);

}
void createChunk(int x, int z) {
	st.chunks[{x, z}] = {};
	auto& c = st.chunks[{x, z}];
	c.x = x;
	c.z = z;
	c.lod = 0;
	if (std::max(std::abs(x), std::abs(z)) >= 3) {
		c.lod = 2;
	}
	int lodLv = 1 << c.lod;
	st.rd.createDescriptorPool(&c.descVol);
	st.rd.createTexture3d(&c.volDensity, c.descVol, nullptr, 
		sc.CHUNK_SIZE_X / lodLv + 1,
		sc.CHUNK_SIZE_ELEVATION / lodLv + 1,
		sc.CHUNK_SIZE_X / lodLv + 1,
		1, 0, AT_IF_SIGNED_FLOAT32_MONO, -1, AT_IU_COMPUTE_OUTPUT);
	c.volDensity->toGeneralLayout();
	st.rd.addStorageImageArrayToDescriptor({ c.volDensity }, c.descVol, 0, -1);
	st.rd.createDescriptorPool(&c.descMesh);
	st.rd.createDescriptorPool(&c.descTreeInst);
	uint32_t chunkSize = std::min(
		static_cast<uint32_t>(sc.MAX_TRIANGLES), 
		sc.CHUNK_SIZE_X * sc.CHUNK_SIZE_ELEVATION * sc.CHUNK_SIZE_X * 15u / (lodLv* lodLv* lodLv));
	using ssboType = std::remove_cv_t<decltype(c.mesh)>;
	using ssboType1 = std::remove_cv_t<decltype(c.treeInstancing)>;
	std::optional<std::function<void(ssboType)>> initFunc = std::nullopt;
	std::optional<std::function<void(ssboType1)>> initFunc2 = std::nullopt;
	st.rd.createShaderStorageBuffer(&c.mesh, chunkSize, 0, c.descMesh, initFunc, -1);
	st.rd.createShaderStorageBuffer(&c.treeInstancing, chunkSize, 0, c.descTreeInst, initFunc2, -1);
	c.treeInstancing->useAsInstancingBuffer();
	c.treeInstancing->setAttrBindingPoint({ 5 });

	c.dseVolPass = {
		{c.descVol,AT_ACDS_STORAGE_IMAGE,0},
		{c.descTreeInst,AT_ACDS_SHADER_STORAGE_BUFFER,0}
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
	float constpc[4] = { (float)x,(float)z,lodLv,sB };

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

inline std::string getWaterAssets(auto x) {
	std::string st(ANTH_ASSET_DIR);
	st += "procterrain\\";
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

void loadWaterTextures() {
	uint32_t width, height, channel;
	uint8_t* rawData;
	AnthemImageLoader loader;
	loader.loadImage(getWaterAssets("matchingNormalMap").c_str(), &width, &height, &channel, &rawData);
	st.rd.createDescriptorPool(&st.descWNormal);
	st.rd.createTexture(&st.waterNormal, st.descWNormal, rawData, width, height, channel, 0, false, false, AT_IF_UNORM_UINT8, -1);
	loader.loadImage(getWaterAssets("waterDuDv").c_str(), &width, &height, &channel, &rawData);
	st.rd.createDescriptorPool(&st.descWDuDv);
	st.rd.createTexture(&st.waterDuDv, st.descWDuDv, rawData, width, height, channel, 0, false, false, AT_IF_UNORM_UINT8, -1);

}

void recordChunkCommands(int x, int z) {
	auto& c = st.chunks[{x, z}];
	st.rd.drAllocateCommandBuffer(&c.cmdVolPass);
	st.rd.drAllocateCommandBuffer(&c.cmdMcPass);

	// Vol Pass
	int lodLv = 1 << c.lod;
	st.rd.drStartCommandRecording(c.cmdVolPass);
	st.rd.drBindComputePipeline(st.passVol[c.lod]->pipeline, c.cmdVolPass);
	st.rd.drBindDescriptorSetCustomizedCompute(c.dseVolPass, st.passVol[c.lod]->pipeline, c.cmdVolPass);
	int gx = sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_X / lodLv;
	int gy = sc.CHUNK_SIZE_ELEVATION / sc.COMPUTE_GROUP_Y / lodLv;
	int gz = sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_Z / lodLv;
	st.rd.drPushConstantsCompute(c.pc, st.passVol[c.lod]->pipeline, c.cmdVolPass);
	st.rd.drComputeDispatch(c.cmdVolPass, gx, gy, gz);
	st.rd.drEndCommandRecording(c.cmdVolPass);

	// Marching Cube Pass
	st.rd.drStartCommandRecording(c.cmdMcPass);
	st.rd.drBindComputePipeline(st.passMarchingCube[c.lod]->pipeline, c.cmdMcPass);
	st.rd.drBindDescriptorSetCustomizedCompute(c.dseMarchingCubePass, st.passMarchingCube[c.lod]->pipeline, c.cmdMcPass);
	st.rd.drPushConstantsCompute(c.pc, st.passMarchingCube[c.lod]->pipeline, c.cmdMcPass);
	st.rd.drComputeDispatch(c.cmdMcPass, gx, gy, gz);
	st.rd.drEndCommandRecording(c.cmdMcPass);

	// Cull Pass
	st.rd.drAllocateCommandBuffer(&c.cmdCullpass);
	st.rd.drStartCommandRecording(c.cmdCullpass);
	st.rd.drBindComputePipeline(st.passCull->pipeline, c.cmdCullpass);
	st.rd.drBindDescriptorSetCustomizedCompute(
	{
		{c.descTreeInst,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descCull,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0},
	}, 
	st.passCull->pipeline, c.cmdCullpass);

	//st.rd.drPushConstantsCompute(c.pc, st.passCull->pipeline, c.cmdCullpass);
	st.rd.drComputeDispatch(c.cmdCullpass, sc.MAX_TREES / sc.COMPUTE_GROUP_CULLING + 1, 1, 1);
	st.rd.drEndCommandRecording(c.cmdCullpass);

	// Submit & Wait
	st.rd.createSemaphore(&c.volPassDone);
	st.rd.createSemaphore(&c.cullSemaphore);
	st.rd.createFence(&c.mcPassDone);
	c.mcPassDone->resetFence();
	st.rd.drSubmitCommandBufferCompQueueGeneralA(c.cmdVolPass, {}, { c.volPassDone },nullptr);
	st.rd.drSubmitCommandBufferCompQueueGeneralA(c.cmdCullpass, { c.volPassDone }, { c.cullSemaphore }, nullptr);
	st.rd.drSubmitCommandBufferCompQueueGeneralA(c.cmdMcPass, { c.cullSemaphore }, {}, c.mcPassDone);
	c.mcPassDone->waitAndReset();
}

void bakeTextures() {
	for (auto i : AT_RANGE2(2)) {
		for (auto j : AT_RANGE2(sc.CSM_LEVELS)) {
			st.shadowFence->resetFence();
			st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.passShadow[j]->getCommandIndex(i), -1, {}, {}, nullptr, {st.shadowSemaphore});
			st.rd.drSubmitCommandBufferGraphicsQueueGeneral2A(st.passShadowTree[j]->getCommandIndex(i), -1, {st.shadowSemaphore}, {AT_SSW_ALL_COMMAND}, st.shadowFence, {});
			st.shadowFence->waitAndReset();
		}
	}

}

void createComputePass() {
	for (uint32_t i : AT_RANGE2(4)) {
		// Vol Pass : Generate Volume Density
		st.passVol[i] = std::make_unique<AnthemComputePassHelper>(&st.rd, 1);
		st.passVol[i]->workGroupSize = { sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_X /(1u<<i), sc.CHUNK_SIZE_ELEVATION / sc.COMPUTE_GROUP_Y / (1u << i), sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_Z / (1u << i) };
		st.passVol[i]->shaderPath.computeShader = getShader("vol.comp");
		st.passVol[i]->pushConstants = { st.demoChunk->pc };
		st.passVol[i]->setDescriptorLayouts(st.demoChunk->dseVolPass);
		st.passVol[i]->buildComputePipeline();

		// Marching Cube Pass : Generate Mesh
		st.passMarchingCube[i] = std::make_unique<AnthemComputePassHelper>(&st.rd, 1);
		st.passMarchingCube[i]->workGroupSize = { sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_X / (1u << i), sc.CHUNK_SIZE_ELEVATION / sc.COMPUTE_GROUP_Y / (1u << i), sc.CHUNK_SIZE_X / sc.COMPUTE_GROUP_Z / (1u << i) };
		st.passMarchingCube[i]->shaderPath.computeShader = getShader("mc.comp");
		st.passMarchingCube[i]->pushConstants = { st.demoChunk->pc };
		st.passMarchingCube[i]->setDescriptorLayouts(st.demoChunk->dseMarchingCubePass);
		st.passMarchingCube[i]->buildComputePipeline();
	}
	st.passCull = std::make_unique<AnthemComputePassHelper>(&st.rd, 1);
	st.passCull->workGroupSize = { sc.COMPUTE_GROUP_CULLING * 1u,1u,1u };
	st.passCull->shaderPath.computeShader = getShader("cull.comp");
	st.passCull->setDescriptorLayouts({ 
		{st.demoChunk->descTreeInst,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descCull,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0},
	});
	st.passCull->buildComputePipeline();
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

void createTreePass() {
	st.passDeferTree = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passDeferTree->shaderPath.fragmentShader = getShader("tree.frag");
	st.passDeferTree->shaderPath.vertexShader = getShader("tree.vert");
	st.passDeferTree->setRenderTargets({ st.gColor, st.gNormal,st.gPos, st.gTangent });
	st.passDeferTree->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32 };
	st.passDeferTree->passOpt.clearColors = { {0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f,0.0f} };
	st.passDeferTree->pipeOpt.blendPreset = { AT_ABP_DEFAULT_TRANSPARENCY,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
	st.passDeferTree->setDescriptorLayouts({
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descPbrBaseTexTree,AT_ACDS_SAMPLER,0},
	});
	st.passDeferTree->vxLayout = nullptr;
	st.passDeferTree->pushConstants = { st.pcTree };
	st.passDeferTree->pipeOpt.vertStageLayout = { st.treeModel.getVertexBuffer(),st.demoChunk->treeInstancing };
	st.passDeferTree->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passDeferTree->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32 };
	st.passDeferTree->passOpt.clearColorAttachmentOnLoad = { false,false,false,false };
	st.passDeferTree->passOpt.clearDepthAttachmentOnLoad = false;
	st.passDeferTree->setDepthFromPass(*st.passDefer);
	st.passDeferTree->buildGraphicsPipeline();
}

void createPreBakingGraphicsPass() {
	//Shadow Pass: Defer
	for (auto i : AT_RANGE2(sc.CSM_LEVELS)) {
		st.passShadow[i] = std::make_unique<AnthemPassHelper>(&st.rd, 2);
		st.passShadow[i]->shaderPath.vertexShader = getShader("shadow.vert");
		st.passShadow[i]->setRenderTargets({});
		st.passShadow[i]->passOpt.colorAttachmentFormats = { };
		st.passShadow[i]->enableDepthSampler = true;
		st.passShadow[i]->pushConstants = { st.pcCSMLight[i] };
		st.passShadow[i]->passOpt.renderPassUsage = AT_ARPAA_DEPTH_STENCIL_ONLY_PASS;
		st.passShadow[i]->passOpt.preserveWritableDepth = true;
		st.passShadow[i]->setDescriptorLayouts({});
		st.passShadow[i]->vxLayout = st.demoChunk->mesh;
		st.passShadow[i]->setDepthBuffer(st.gCSMDepth[i]);
		st.passShadow[i]->viewport = st.depthViewport;
		st.passShadow[i]->buildGraphicsPipeline();

		st.passShadowTree[i] = std::make_unique<AnthemPassHelper>(&st.rd, 2);
		st.passShadowTree[i]->shaderPath.vertexShader = getShader("shadowtree.vert");
		st.passShadowTree[i]->shaderPath.fragmentShader = getShader("shadowtree.frag");
		st.passShadowTree[i]->setRenderTargets({});
		st.passShadowTree[i]->passOpt.colorAttachmentFormats = { };
		st.passShadowTree[i]->enableDepthSampler = true;
		st.passShadowTree[i]->pushConstants = { st.pcCSMLight[0] };
		st.passShadowTree[i]->viewport = st.depthViewport;
		st.passShadowTree[i]->passOpt.renderPassUsage = AT_ARPAA_DEPTH_STENCIL_ONLY_PASS;
		st.passShadowTree[i]->setDescriptorLayouts({
			{st.descPbrBaseTexTree,AT_ACDS_SAMPLER,0},
		});
		st.passShadowTree[i]->vxLayout = nullptr;
		st.passShadowTree[i]->pipeOpt.vertStageLayout = { st.treeModel.getVertexBuffer(),st.demoChunk->treeInstancing };
		st.passShadowTree[i]->setDepthFromPass(*st.passShadow[i]);
		st.passShadowTree[i]->passOpt.clearDepthAttachmentOnLoad = false;
		st.passShadowTree[i]->buildGraphicsPipeline();
	}
	
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

	// Ground Pass
	st.passGround = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passGround->shaderPath.fragmentShader = getShader("ground.frag");
	st.passGround->shaderPath.vertexShader = getShader("ground.vert");
	st.passGround->setRenderTargets({ st.gColor, st.gNormal,st.gPos, st.gTangent });
	st.passGround->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32 };
	st.passGround->passOpt.clearColors = { {0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f,0.0f} };
	st.passGround->pipeOpt.blendPreset = { AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
	st.passGround->setDescriptorLayouts({
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0}
		});
	st.passGround->vxLayout = sf.vx;
	st.passGround->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passGround->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32 };
	st.passGround->passOpt.clearColorAttachmentOnLoad = { false,false,false,false };
	st.passGround->setDepthFromPass(*st.passDefer);
	st.passGround->passOpt.clearDepthAttachmentOnLoad = false;
	st.passGround->buildGraphicsPipeline();


	// AO Pass
	st.passAO = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passAO->shaderPath.fragmentShader = getShader("ao.frag");
	st.passAO->shaderPath.vertexShader = getShader("ao.vert");
	st.passAO->setRenderTargets({st.gAO });
	st.passAO->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32_MONO };
	st.passAO->setDescriptorLayouts({
		{st.descGNormal,AT_ACDS_SAMPLER,0},
		{st.descGPos,AT_ACDS_SAMPLER,0},
		{st.descAOSamples,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.passAO->vxLayout = canvas.vx;
	st.passAO->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passAO->buildGraphicsPipeline();

	// Water Pass I: Get Mask for Underwater stuffs
	st.passSurface = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.passSurface->shaderPath.fragmentShader = getShader("water.frag");
	st.passSurface->shaderPath.vertexShader = getShader("water.vert");
	st.passSurface->setRenderTargets({ st.gUnderWaterMask,st.gWaterUV });
	st.passSurface->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32,AT_IF_SIGNED_FLOAT32 };
	st.passSurface->passOpt.clearColors = { {0,0,0,0},{0,0,0,0} };
	st.passSurface->passOpt.storeDepthValues = true;
	st.passSurface->pipeOpt.enableDepthWriting = false;
	st.passSurface->passOpt.clearDepthAttachmentOnLoad = false;
	st.passSurface->passOpt.clearColorAttachmentOnLoad = { true,true };
	st.passSurface->setDepthFromPass(*st.passDefer);
	st.passSurface->setDescriptorLayouts({
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0},
	});
	st.passSurface->vxLayout = sf.vx;
	st.passSurface->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passSurface->pipeOpt.blendPreset = {AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
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
	//st.passDeferBlend->pushConstants = { st.pcCSMLight[0] };
	st.passDeferBlend->setDescriptorLayouts({
		{st.descGColor,AT_ACDS_SAMPLER,0},
		{st.descGNormal,AT_ACDS_SAMPLER,0},
		{st.descGPos,AT_ACDS_SAMPLER,0},
		{st.descGAO,AT_ACDS_SAMPLER,0},
		{st.descCamera,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descGUnderwaterMask,AT_ACDS_SAMPLER,0},
		{st.descBox,AT_ACDS_SAMPLER,0},
		{st.descWNormal,AT_ACDS_SAMPLER,0},
		{st.descWDuDv,AT_ACDS_SAMPLER,0},
		{st.descGWaterUV, AT_ACDS_SAMPLER,0},
		{st.descCSMDepth,AT_ACDS_SAMPLER,0},
		{st.descCSMData,AT_ACDS_UNIFORM_BUFFER,0}
	});

	st.passDeferBlend->vxLayout = canvas.vx;
	st.passDeferBlend->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.passDeferBlend->passOpt.colorAttachmentFormats = { AT_IF_UNORM_UINT8 };
	st.passDeferBlend->buildGraphicsPipeline();

	// Global Fog Pass
	st.passFog = std::make_unique<AnthemGlobalFog>(&st.rd, 2);
	st.passFog->addInput({
		{st.descGDeferBlend,AT_ACDS_SAMPLER,0},
		{st.descGPos,AT_ACDS_SAMPLER,0},
		{st.descGlobalFog,AT_ACDS_UNIFORM_BUFFER,0},
	});
	st.passFog->prepare(true);

	// FXAA Pass
	st.passFXAA = std::make_unique<AnthemFXAA>(&st.rd, 2);
	st.passFXAA->addInput({
		{st.passFog->getColorAttachmentDescId(0),AT_ACDS_SAMPLER,0},
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
	st.rd.createColorAttachmentImage(&st.gAO, st.descGAO, 0, AT_IF_SIGNED_FLOAT32_MONO, false, -1);
	st.rd.createDescriptorPool(&st.descAOSamples);
	st.rd.createUniformBuffer(&st.aoSamples, 0, st.descAOSamples, -1);
	st.rd.createDescriptorPool(&st.descGTangent);
	st.rd.createColorAttachmentImage(&st.gTangent, st.descGTangent, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descGDeferBlend);
	st.rd.createColorAttachmentImage(&st.gDeferBlend, st.descGDeferBlend, 0, AT_IF_UNORM_UINT8, false, -1);
	st.rd.createDescriptorPool(&st.descGUnderwaterMask);
	st.rd.createColorAttachmentImage(&st.gUnderWaterMask, st.descGUnderwaterMask, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descGWaterUV);
	st.rd.createColorAttachmentImage(&st.gWaterUV, st.descGWaterUV, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descDebug);
	st.rd.createColorAttachmentImage(&st.gDebug, st.descDebug, 0, AT_IF_SIGNED_FLOAT32, false, -1);


	//Sync
	st.rd.createSemaphore(&st.shadowSemaphore);
	st.rd.createFence(&st.shadowFence);
}

void createWaterGeometry() {
	st.rd.createVertexBuffer(&sf.vx);
	st.rd.createIndexBuffer(&sf.ix);

	sf.vx->setTotalVertices(4);
	sf.vx->insertData(0, { -sc.WATER_WIDTH,sc.WATER_ELEVATION,-sc.WATER_WIDTH,1 },{0,1,0,0}, {1,0,0,0}, {0,0,0,0});
	sf.vx->insertData(1, { sc.WATER_WIDTH,sc.WATER_ELEVATION,-sc.WATER_WIDTH,1 }, { 0,1,0,0 }, { 1,0,0,0 }, { 1,0,0,0 });
	sf.vx->insertData(2, { sc.WATER_WIDTH,sc.WATER_ELEVATION,sc.WATER_WIDTH,1 }, { 0,1,0,0 }, { 1,0,0,0 }, { 1,1,0,0 });
	sf.vx->insertData(3, { -sc.WATER_WIDTH,sc.WATER_ELEVATION,sc.WATER_WIDTH,1 }, { 0,1,0,0 }, { 1,0,0,0 }, { 0,1,0,0 });

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
	for (auto i : AT_RANGE2(sc.CSM_LEVELS)) {
		st.passShadow[i]->recordCommands([&](uint32_t x) {
			st.rd.drPushConstants(st.pcCSMLight[i], st.passShadow[i]->pipeline, x);
			injectCommands(x);
		});
	}
	st.passGround->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(sf.vx, x);
		st.rd.drBindIndexBuffer(sf.ix, x);
		st.rd.drDraw(sf.ix->getIndexCount(), x);
	});
	st.passAO->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(canvas.vx, x);
		st.rd.drBindIndexBuffer(canvas.ix, x);
		st.rd.drDraw(6, x);
	});
	st.passDeferBlend->recordCommands([&](uint32_t x) {
		//st.rd.drPushConstants(st.pcCSMLight[0], st.passDeferBlend->pipeline, x);
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
	st.passFog->recordCommandOffscreen();
	st.passFXAA->recordCommand();

	st.seq[0] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.seq[0]->markGraphicsOnly();
	st.seq[1] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.seq[1]->markGraphicsOnly();

	st.seq[0]->setSequence({
		//{st.passShadow->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{ st.passDefer->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.passGround->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{ st.passDeferTree->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{ st.passSurface->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{  st.passSkybox->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{ st.passAO->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.passDeferBlend->getCommandIndex(0),ATC_ASCE_GRAPHICS},
		{st.passFog->getCommandIdx(0),ATC_ASCE_GRAPHICS},
		{ st.passFXAA->getCommandIdx(0), ATC_ASCE_GRAPHICS},
	});
	st.seq[1]->setSequence({
		//{st.passShadow->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{ st.passDefer->getCommandIndex(1), ATC_ASCE_GRAPHICS} ,
		{st.passGround->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{st.passDeferTree->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{st.passSurface->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{st.passSkybox->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{ st.passAO->getCommandIndex(1), ATC_ASCE_GRAPHICS},
		{ st.passDeferBlend->getCommandIndex(1),ATC_ASCE_GRAPHICS},
		{st.passFog->getCommandIdx(1),ATC_ASCE_GRAPHICS},
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
	float tick[4];

	vc[0] = camPos[0];
	vc[1] = camPos[1];
	vc[2] = camPos[2];
	vc[3] = 0;
	tick[0] = static_cast<float>(glfwGetTime());
	
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);
	st.ubCamera->specifyUniforms(pm, vm, lm,vc,tick);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.ubCamera->updateBuffer(i);
	}

	// Global fogs
	float fogParams[4];
	fogParams[0] = sc.FOG_SCATTER;
	fogParams[1] = sc.FOG_HEIGHT_ATTENUATION;
	fogParams[2] = sc.FOG_ELEVATION;
	st.ubGlobalFog->specifyUniforms(fogParams);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.ubGlobalFog->updateBuffer(i);
	}
}

void createAllChunks() {
	for (int i = -sc.WORLD_SIZE; i <= sc.WORLD_SIZE; i++) {
		for (int j = sc.WORLD_SIZENZ; j <= sc.WORLD_SIZEZ; j++) {
			createChunk(i, j);
		}
	}
}

void recordAllChunks() {
	for (int i = -sc.WORLD_SIZE; i <= sc.WORLD_SIZE; i++) {
		for (int j = sc.WORLD_SIZENZ; j <= sc.WORLD_SIZEZ; j++) {
			recordChunkCommands(i, j);
		}
	}
	st.passDeferTree->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBufferEx2(st.treeModel.getVertexBuffer(), st.culledTrees, 0, x);
		st.rd.drBindIndexBuffer(st.treeModel.getIndexBuffer(), x);
		auto insts = st.culledTrees->getAtomicCounter(0);
		for (auto i : AT_RANGE(0,2)) {
			st.rd.drPushConstants(st.pcTree, st.passDeferTree->pipeline, x);
			st.rd.drDrawInstancedAll(st.treeModel.drawVC[i], insts, st.treeModel.drawFI[i], st.treeModel.drawVO[i], 0, x);
		}
	});
	for (auto i : AT_RANGE2(sc.CSM_LEVELS)) {
		st.passShadowTree[i]->recordCommands([&](uint32_t x) {
			st.rd.drBindVertexBufferEx2(st.treeModel.getVertexBuffer(), st.culledTrees, 0, x);
			st.rd.drBindIndexBuffer(st.treeModel.getIndexBuffer(), x);
			auto insts = st.culledTrees->getAtomicCounter(0);
			for (auto i : AT_RANGE(0, 2)) {
				st.rd.drPushConstants(st.pcCSMLight[i], st.passShadowTree[i]->pipeline, x);
				st.rd.drDrawInstancedAll(st.treeModel.drawVC[i], insts, st.treeModel.drawFI[i], st.treeModel.drawVO[i], 0, x);
			}
		});
	}

}
void createAllIndexBuffer() {
	for (int i = -sc.WORLD_SIZE; i <= sc.WORLD_SIZE; i++) {
		for (int j = sc.WORLD_SIZENZ; j <= sc.WORLD_SIZEZ; j++) {
			createIndexBuffer(i, j);
		}
	}
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
	st.frm.record();
	std::stringstream ss;
	ss << "FPS:";
	ss << st.frm.getFrameRate();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Control Panel");
	ImGui::Text(ss.str().c_str());
	ImGui::SliderFloat("Fog Scatter", &sc.FOG_SCATTER, 0.0f, 0.0010f,"%.4f");
	ImGui::SliderFloat("Fog Height Attn", &sc.FOG_HEIGHT_ATTENUATION, 0.0f, 0.05f, "%.4f");
	ImGui::SliderFloat("Fog Elevation", &sc.FOG_ELEVATION, 0.0f, 250.0f);
	ImGui::End();
}

void drawLoop() {
	static int cur = 0;

	updateUniform();
	uint32_t imgIdx = 0;
	st.rd.drPrepareFrame(cur, &imgIdx);
	prepareImguiFrame();
	st.seq[cur]->executeCommandToStage(imgIdx, false, true, st.passFXAA->getSwapchainFb());
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}


int main() {
	initialize();
	setupImgui();
	initCSM();
	initCull();
	prepareGlobalFog();
	createUniform();
	createTextures();
	loadWaterTextures();
	loadTree();
	
	createWaterGeometry();
	createGeometrySkybox();
	prepareAOSamples();
	prepareDeferCanvas();
	createAllChunks();
	createComputePass();
	createPreBakingGraphicsPass();
	createGraphicsPass();
	
	createTreePass();
	createSkyPass();

	st.rd.registerPipelineSubComponents();
	recordAllChunks();
	ANTH_LOGI("Chunk created");

	createAllIndexBuffer();
	recordDrawCommand();

	updateUniform();
	bakeTextures();

	st.rd.setDrawFunction(drawLoop);
	st.rd.startDrawLoopDemo();
	st.rd.exDestroyImgui();
	st.rd.finalize();

	return 0;
}