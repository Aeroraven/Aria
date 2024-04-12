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

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "toon\\toon.";
	st += x;
	st += ".hlsl.spv";
	return st;
}

struct Stage {
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;

	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemCamera camLight = AnthemCamera(AT_ACPT_ORTHO);
	AnthemFrameRateMeter fpsMeter = AnthemFrameRateMeter(10);

	std::vector<AnthemGLTFLoaderParseResult> gltfModel;
	AnthemGLTFLoaderTexParseResult gltfTex;
	AnthemSimpleModelIntegrator model;
	AnthemDescriptorPool* descPbrBaseTex = nullptr;
	AnthemImage** pbrBaseTex = nullptr;
	std::vector<std::string> requiredTex;

	std::unique_ptr<AnthemPassHelper> mainPass;
	std::unique_ptr<AnthemPassHelper> outlinePass;
	std::unique_ptr<AnthemSequentialCommand> passSeq[2];

	AnthemUniformBufferImpl< AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uniform;
	AnthemDescriptorPool* descUni;
}st;


void initialize() {
	st.cfg.demoName = "31. Toon Shader";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);

	st.camera.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 0.9, -1.5);
	st.camera.specifyFrontEyeRay(0, 0, 1);
}

void loadModel() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\kirara\\kirara-2.gltf").c_str());
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

void prepareMainPass() {
	st.mainPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.mainPass->shaderPath.vertexShader = getShader("main.vert");
	st.mainPass->shaderPath.fragmentShader = getShader("main.frag");
	st.mainPass->vxLayout = st.model.getVertexBuffer();
	st.mainPass->passOpt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.mainPass->setDescriptorLayouts({
			{st.descUni,AT_ACDS_UNIFORM_BUFFER,0},
		});
	}
	st.mainPass->buildGraphicsPipeline();
}

void recordCommand() {
	st.mainPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.model.getVertexBuffer(), x);
		st.rd.drBindIndexBuffer(st.model.getIndexBuffer(), x);
		st.rd.drDraw(st.model.getIndexBuffer()->getIndexCount(), x);
		});

	st.passSeq[0] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.passSeq[1] = std::make_unique<AnthemSequentialCommand>(&st.rd);

	st.passSeq[0]->setSequence({
		{ st.mainPass->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		});
	st.passSeq[1]->setSequence({
		{ st.mainPass->getCommandIndex(1), ATC_ASCE_GRAPHICS} ,
		});
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

	st.uniform->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniform->updateBuffer(i);
	}
}

void drawCall() {
	static int cur = 0;
	updateUniform();
	uint32_t imgIdx = 0;
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.passSeq[cur]->executeCommandToStage(imgIdx, false, true, st.mainPass->getSwapchainBuffer());
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}


int main() {
	initialize();
	prepareMainPass();

	st.rd.registerPipelineSubComponents();
	recordCommand();

	st.rd.setDrawFunction(drawCall);
	st.rd.startDrawLoopDemo();

	st.rd.exDestroyImgui();
	st.rd.finalize();
	return 0;
}