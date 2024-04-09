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
#include "../include/components/postprocessing/AnthemGlobalFog.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::Components::PassHelper;
using namespace Anthem::Components::Postprocessing;
using namespace Anthem::Components::Math;
using namespace Anthem::External;
using namespace Anthem::Core;

#define DCONST static constexpr const
struct StageConstants {
	DCONST int GRID_SIZE = 640;
	DCONST float SURFACE_WIDTH = 128;
	float FBM_FREQ_FACTOR = 1.18;
	float FBM_AMPL_FACTOR = 0.75;
	float BASE_FREQ = 1.5;
	float BASE_AMPL = 0.2;
	DCONST float PHASE_RANGE = 1;
	DCONST int NUM_WAVES = 32;
	DCONST float SUN_POS[4] = { 0,-1,-1,0 };
	DCONST float SKYBOX_SIZE = 64;
	float DOMAIN_WARP = 1.0;
	int USE_WAVES = 32;
	float SCATTER_FACTOR = 0.015;
	float HEIGHT_ATTN = 0.3;
}sc;
#undef DCONST


struct Stage {
	AnthemFrameRateMeter fpsMeter = AnthemFrameRateMeter(10);
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camMain = AnthemCamera(AT_ACPT_PERSPECTIVE);

	AnthemDescriptorPool* descUni;
	AnthemUniformBufferImpl<
		AtUniformMatf<4>,
		AtUniformMatf<4>,
		AtUniformMatf<4>,
		AtUniformVecf<4>, //(time,fbm_freq,fbm_ampl,0)
		AtUniformVecf<4>, //(base_freq,base_ampl,fbm_freq,fbm_ampl
		AtUniformVecf<4>, //(sundir)
		AtUniformVecf<4>, //(campos)
		AtUniformVecf<4>, //(domainWarps,numWaves)
		AtUniformVecfArray<4, sc.NUM_WAVES>	//(dirPosition,dirTemporal)
	>* ubuf;

	AnthemDescriptorPool* descUniFog;
	AnthemUniformBufferImpl <
		AtUniformMatf<4>
	>* ubufFog;

	AnthemVertexBufferImpl<
		AtAttributeVecf<4>
	>* vx;
	AnthemIndexBuffer* ix;

	AnthemDescriptorPool* descCanv;
	AnthemImage* canvas;
	AnthemDescriptorPool* descPos;
	AnthemImage* pos;

	std::unique_ptr<AnthemPassHelper> mainPass;
	std::unique_ptr<AnthemPassHelper> skyPass;

	std::unique_ptr<AnthemGlobalFog> idPass;
	std::unique_ptr<AnthemSequentialCommand> passSeq[2];

	float dirA[sc.NUM_WAVES];
	float phase[sc.NUM_WAVES];
	bool dirInit = false;

	// Skybox
	AnthemVertexBufferImpl<AtAttributeVecf<4>>* sbox;
	AnthemIndexBuffer* ixBox;

	// Texture & Viewport
	AnthemDescriptorPool* descBox;
	AnthemImageCubic* texSkybox;

}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "sine\\sine.";
	st += x;
	st += ".hlsl.spv";
	return st;
}
inline std::string getSkyboxTex(auto x) {
	std::string st(ANTH_ASSET_DIR);
	st += "skybox2\\";
	st += x;
	st += ".png";
	return st;
}

void initialize() {
	st.cfg.demoName = "30. Sine Waves";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.camMain.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.01f, 1000.0f, 1.0f * rdW / rdH);
	st.camMain.specifyPosition(0, 1, 0);
	st.camMain.specifyFrontEyeRay(0, 0, 1);
}

void createGeometry() {
	st.rd.createVertexBuffer(&st.vx);
	st.rd.createIndexBuffer(&st.ix);
	st.vx->setTotalVertices(sc.GRID_SIZE * sc.GRID_SIZE);
	std::vector<uint32_t> idx;
#define PX(a,b) ((a)*sc.GRID_SIZE+(b))
	for (auto i : AT_RANGE2(sc.GRID_SIZE)) {
		for (auto j : AT_RANGE2(sc.GRID_SIZE)) {
			st.vx->insertData(PX(i,j), {
				(i / (sc.GRID_SIZE - 1.0f) - 0.5f)* sc.SURFACE_WIDTH,
				0,
				(j / (sc.GRID_SIZE - 1.0f) - 0.5f)* sc.SURFACE_WIDTH,
				1});
			if (i != sc.GRID_SIZE - 1 && j != sc.GRID_SIZE - 1) {
				idx.push_back(PX(i, j));
				idx.push_back(PX(i + 1, j));
				idx.push_back(PX(i + 1, j + 1));
				idx.push_back(PX(i + 1, j + 1));
				idx.push_back(PX(i, j + 1));
				idx.push_back(PX(i, j));
			}
		}
	}
#undef PX
	st.ix->setIndices(idx);
}

void createGeometrySkybox() {
	auto c = sc.SKYBOX_SIZE;
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

void createRenderTargets() {
	st.rd.createDescriptorPool(&st.descCanv);
	st.rd.createColorAttachmentImage(&st.canvas, st.descCanv, 0, AT_IF_SIGNED_FLOAT32, false, -1);
	st.rd.createDescriptorPool(&st.descPos);
	st.rd.createColorAttachmentImage(&st.pos, st.descPos, 0, AT_IF_SIGNED_FLOAT32, false, -1);
}

void createUniform() {
	st.rd.createDescriptorPool(&st.descUni);
	st.rd.createUniformBuffer(&st.ubuf, 0, st.descUni, -1);
	st.rd.createDescriptorPool(&st.descUniFog);
	st.rd.createUniformBuffer(&st.ubufFog, 0, st.descUniFog, -1);
}
void updateUniform() {
	//Camera
	AtMatf4 proj, view, local, pv;
	st.camMain.getProjectionMatrix(proj);
	st.camMain.getViewMatrix(view);
	local = AnthemLinAlg::axisAngleRotationTransform3<float, float>({ 0.0f,1.0f,0.0f }, glfwGetTime() * 0.00f);

	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);

	//Param
	float dstat[4 * sc.NUM_WAVES];
	float freqAmpl[4] = { sc.BASE_FREQ,sc.BASE_AMPL,sc.FBM_FREQ_FACTOR,sc.FBM_AMPL_FACTOR };
	float timer[4] = { glfwGetTime() * 1.0f };
	float camPosR[4] = { 0,1,0,0 };
	float sunDir[4] = { sc.SUN_POS[0],sc.SUN_POS[1],sc.SUN_POS[2],0 };
	float attrX[4] = { sc.DOMAIN_WARP,sc.USE_WAVES,0,0 };

	float fogAttr[4] = { sc.SCATTER_FACTOR,sc.HEIGHT_ATTN,0,0 };
	if (st.dirInit == false) {
		st.dirInit = true;
		for (auto i : AT_RANGE2(sc.NUM_WAVES)) {
			st.dirA[i] = AnthemLinAlg::randomNumber<float>() * AT_PI * 2;
			st.phase[i] = AnthemLinAlg::randomNumber<float>() * sc.PHASE_RANGE + 1.0;
		}
	}
	for (auto i : AT_RANGE2(sc.NUM_WAVES)) {
		dstat[i * 4 + 0] = std::cos(st.dirA[i]);
		dstat[i * 4 + 1] = std::sin(st.dirA[i]);
		dstat[i * 4 + 2] = st.phase[i];
		dstat[i * 4 + 3] = 0;
	}
	st.ubufFog->specifyUniforms(fogAttr);
	st.ubuf->specifyUniforms(pm, vm, lm, timer, freqAmpl,sunDir, camPosR, attrX, dstat);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.ubuf->updateBuffer(i);
		st.ubufFog->updateBuffer(i);
	}
}

void createMainPass() {
	st.mainPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.mainPass->shaderPath.vertexShader = getShader("main.vert");
	st.mainPass->shaderPath.fragmentShader = getShader("main.frag");
	st.mainPass->vxLayout = st.vx;
	st.mainPass->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.mainPass->setRenderTargets({ st.canvas,st.pos });
	st.mainPass->setDepthFromPass(*st.skyPass.get());
	st.mainPass->passOpt.clearColorAttachmentOnLoad = { false,false };
	st.mainPass->passOpt.clearDepthAttachmentOnLoad = false;
	st.mainPass->passOpt.clearColors = { {0,0,0,1},{0,0,0,1} };
	st.mainPass->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32 };
	st.mainPass->pipeOpt.blendPreset = { AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
	st.mainPass->setDescriptorLayouts({
		{st.descUni,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descBox,AT_ACDS_SAMPLER,0}
	});
	st.mainPass->buildGraphicsPipeline();
}

void createSkyPass() {
	st.skyPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.skyPass->shaderPath.vertexShader = getShader("sky.vert");
	st.skyPass->shaderPath.fragmentShader = getShader("sky.frag");
	st.skyPass->vxLayout = st.sbox;
	st.skyPass->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.skyPass->setRenderTargets({ st.canvas,st.pos });
	st.skyPass->passOpt.clearColorAttachmentOnLoad = { true,true };
	st.skyPass->passOpt.clearColors = { {0,0,0,1},{0,0,0,1} };
	st.skyPass->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32, AT_IF_SIGNED_FLOAT32 };
	st.skyPass->pipeOpt.blendPreset = { AT_ABP_NO_BLEND,AT_ABP_NO_BLEND };
	st.skyPass->setDescriptorLayouts({
		{st.descUni,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descBox,AT_ACDS_SAMPLER,0},
	});
	st.skyPass->buildGraphicsPipeline();
}

void recordCommand() {
	st.mainPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.vx, x);
		st.rd.drBindIndexBuffer(st.ix, x);
		st.rd.drDraw(st.ix->getIndexCount(), x);
	});

	st.skyPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.sbox, x);
		st.rd.drBindIndexBuffer(st.ixBox, x);
		st.rd.drDraw(st.ixBox->getIndexCount(), x);
	});
	st.idPass->recordCommand();

	st.passSeq[0] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.passSeq[1] = std::make_unique<AnthemSequentialCommand>(&st.rd);

	st.passSeq[0]->setSequence({ 
		{ st.skyPass->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.mainPass->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.idPass->getCommandIdx(0), ATC_ASCE_GRAPHICS}
	});
	st.passSeq[1]->setSequence({ 
		{ st.skyPass->getCommandIndex(1), ATC_ASCE_GRAPHICS},
		{ st.mainPass->getCommandIndex(1), ATC_ASCE_GRAPHICS} ,
		{ st.idPass->getCommandIdx(1), ATC_ASCE_GRAPHICS}
	});
}

void createIdPass() {
	st.idPass = std::make_unique<AnthemGlobalFog>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.idPass->addInput({ 
		{ st.descCanv,AT_ACDS_SAMPLER,0},
		{ st.descPos,AT_ACDS_SAMPLER,0},
		{ st.descUniFog,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.idPass->prepare();
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

	ImGui::SliderFloat("Frequency", &sc.BASE_FREQ, 1.0f, 5.0f);
	ImGui::SliderFloat("Amplitude", &sc.BASE_AMPL, 0.01f, 0.5f);
	ImGui::SliderFloat("FBM Freq Scaler", &sc.FBM_FREQ_FACTOR, 1.0f, 2.0f);
	ImGui::SliderFloat("FBM Ampl Scaler", &sc.FBM_AMPL_FACTOR, 0.0f, 1.0f);
	ImGui::SliderFloat("FBM Domain Warp", &sc.DOMAIN_WARP, 0.0f, 1.0f);
	ImGui::SliderInt("Num Waves", &sc.USE_WAVES, 1, 32);
	ImGui::SliderFloat("Fog Scatter", &sc.SCATTER_FACTOR, 0.0f, 0.5f);
	ImGui::SliderFloat("Fog Attenuation", &sc.HEIGHT_ATTN, 0.0f, 2.0f);

	ImGui::End();
}


void drawCall() {
	static int cur = 0;
	updateUniform();
	uint32_t imgIdx = 0;
	st.rd.drPrepareFrame(cur, &imgIdx);
	prepareImguiFrame();
	st.passSeq[cur]->executeCommandToStage(imgIdx, false, true, st.idPass->getSwapchainFb());
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}

int main() {
	initialize();
	setupImgui();
	createRenderTargets();
	createGeometry();
	createUniform();
	createGeometrySkybox();
	createTextures();

	createSkyPass();
	createMainPass();
	createIdPass();

	st.rd.registerPipelineSubComponents();
	recordCommand();

	st.rd.setDrawFunction(drawCall);
	st.rd.startDrawLoopDemo();
	st.rd.exDestroyImgui();
	st.rd.finalize();
	return 0;
}