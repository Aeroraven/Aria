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
#include "../include/components/postprocessing/AnthemFXAA.h"

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
	DCONST int GRID_SIZE = 320;
	DCONST float SURFACE_WIDTH = 16;
	DCONST float FBM_FREQ_FACTOR = 1.18;
	DCONST float FBM_AMPL_FACTOR = 0.75;
	DCONST float BASE_FREQ = 1.5;
	DCONST float BASE_AMPL = 0.2;
	DCONST float PHASE_RANGE = 1;
	DCONST int NUM_WAVES = 32;
	DCONST float SUN_POS[4] = { 0,-1,-1,0 };
	DCONST float SKYBOX_SIZE = 1.0;
}sc;
#undef DCONST


struct Stage {
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
		AtUniformVecfArray<4, sc.NUM_WAVES>	//(dirPosition,dirTemporal)
	>* ubuf;

	AnthemVertexBufferImpl<
		AtAttributeVecf<4>
	>* vx;
	AnthemIndexBuffer* ix;

	std::unique_ptr<AnthemPassHelper> mainPass;
	std::unique_ptr<AnthemPassHelper> skyPass;
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
	st.camMain.specifyPosition(0, 1, -2);
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

void createUniform() {
	st.rd.createDescriptorPool(&st.descUni);
	st.rd.createUniformBuffer(&st.ubuf, 0, st.descUni, -1);
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
	float camPosR[4] = { 0,1,-2,0 };
	float sunDir[4] = { sc.SUN_POS[0],sc.SUN_POS[1],sc.SUN_POS[2],0 };
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

	st.ubuf->specifyUniforms(pm, vm, lm, timer, freqAmpl,sunDir, camPosR, dstat);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.ubuf->updateBuffer(i);
	}
}

void createMainPass() {
	st.mainPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.mainPass->shaderPath.vertexShader = getShader("main.vert");
	st.mainPass->shaderPath.fragmentShader = getShader("main.frag");
	st.mainPass->vxLayout = st.vx;
	st.mainPass->passOpt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	st.mainPass->passOpt.clearColorAttachmentOnLoad = { false };
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
	st.skyPass->passOpt.renderPassUsage = AT_ARPAA_FINAL_PASS;
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

	st.passSeq[0] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.passSeq[1] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.passSeq[0]->setSequence({ 
		{ st.skyPass->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.mainPass->getCommandIndex(0), ATC_ASCE_GRAPHICS} 
	});
	st.passSeq[1]->setSequence({ 
		{ st.skyPass->getCommandIndex(1), ATC_ASCE_GRAPHICS},
		{ st.mainPass->getCommandIndex(1), ATC_ASCE_GRAPHICS} 
	});
}


void drawCall() {
	static int cur = 0;
	updateUniform();
	uint32_t imgIdx = 0;
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.passSeq[cur]->executeCommandToStage(imgIdx, false);
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}

int main() {
	initialize();
	createGeometry();
	createUniform();
	createGeometrySkybox();
	createTextures();

	createSkyPass();
	createMainPass();

	st.rd.registerPipelineSubComponents();
	recordCommand();

	st.rd.setDrawFunction(drawCall);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();
	return 0;
}