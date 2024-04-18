#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/components/camera/AnthemOrbitControl.h"
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
#include "../include/components/postprocessing/AnthemFXAA.h"

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
	std::unique_ptr<AnthemFXAA> fxaaPass;
	std::unique_ptr<AnthemSequentialCommand> passSeq[2];

	AnthemUniformBufferImpl< AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>, AtUniformVecf<4>>* uniform;
	AnthemDescriptorPool* descUni;

	AnthemDescriptorPool* descCanv;
	AnthemImage* canvas;
	float aspect;

	std::function<void(int, int, int, int)> keyController;
	std::function<void(double, double)> mouseMoveController;
	std::function<void(int, int, int)> mouseController;
	AnthemOrbitControl orbitControl;

	uint32_t* mipmapCommand = nullptr;
}st;


void initialize() {
	st.cfg.demoName = "31. Toon Shader";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.aspect = 1.0f * rdW / rdH;

	st.camera.specifyFrustum((float)AT_PI * 1.0f / 3.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, -0.2, -1.6);
	st.camera.specifyFrontEyeRay(0, 0, 1);

	st.rd.createDescriptorPool(&st.descUni);
	st.rd.createUniformBuffer(&st.uniform, 0, st.descUni, -1);

	st.rd.createDescriptorPool(&st.descCanv);
	st.rd.createColorAttachmentImage(&st.canvas, st.descCanv, 0, AT_IF_SIGNED_FLOAT32, false, -1, true);

	st.keyController = st.camera.getKeyboardController(0.01f);
	st.rd.ctSetKeyBoardController(st.keyController);

	st.mouseController = st.orbitControl.getMouseController(0.01f);
	st.mouseMoveController = st.orbitControl.getMouseMoveController(0.01f);
	st.rd.ctSetMouseController(st.mouseController);
	st.rd.ctSetMouseMoveController(st.mouseMoveController);
	st.orbitControl.specifyTranslation(0, -1.1, 0);
}

void loadModel() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\kirara\\kirara-2.gltf").c_str());
	loader.parseModel(config, st.gltfModel, &st.gltfTex);
	std::vector<AnthemUtlSimpleModelStruct> rp;
	for (auto i = 0; auto & p : st.gltfModel) {
		if (i != 26 && i != 28 && i != 29) {
			rp.push_back(p);
		}
		i++;
	}
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
	st.mainPass->setRenderTargets({ st.canvas });
	st.mainPass->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.mainPass->passOpt.clearColors = {{1.0f,1.0f,1.0f,1.0f}};
	st.mainPass->pipeOpt.enableCullMode = false;
	st.mainPass->pipeOpt.cullMode = AT_ACM_FRONT;
	st.mainPass->pipeOpt.frontFace = AT_AFF_COUNTER_CLOCKWISE;
	st.mainPass->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };

	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.mainPass->setDescriptorLayouts({
			{st.descUni,AT_ACDS_UNIFORM_BUFFER,0},
			{st.descPbrBaseTex,AT_ACDS_SAMPLER,0},
		});
	}
	st.mainPass->buildGraphicsPipeline();
}

void prepareOutlinePass() {
	st.outlinePass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.outlinePass->shaderPath.vertexShader = getShader("outline.vert");
	st.outlinePass->shaderPath.fragmentShader = getShader("outline.frag");
	st.outlinePass->vxLayout = st.model.getVertexBuffer();
	st.outlinePass->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.outlinePass->passOpt.clearColorAttachmentOnLoad = { false };
	st.outlinePass->passOpt.clearDepthAttachmentOnLoad = false;
	st.outlinePass->passOpt.clearColors = { {1.0f,1.0f,1.0f,1.0f} };
	st.outlinePass->setDepthFromPass(*st.mainPass);
	st.outlinePass->pipeOpt.enableCullMode = true;
	st.outlinePass->pipeOpt.cullMode = AT_ACM_BACK;
	st.outlinePass->pipeOpt.frontFace = AT_AFF_COUNTER_CLOCKWISE;
	st.outlinePass->setRenderTargets({ st.canvas });
	st.outlinePass->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };

	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.outlinePass->setDescriptorLayouts({
			{st.descUni,AT_ACDS_UNIFORM_BUFFER,0},
			{st.descPbrBaseTex,AT_ACDS_SAMPLER,0},
		});
	}
	st.outlinePass->buildGraphicsPipeline();
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
	ImGui::End();
}


void preparePostAA() {
	st.fxaaPass = std::make_unique<AnthemFXAA>(&st.rd, 2);
	st.fxaaPass->addInput({
		{st.descCanv,AT_ACDS_SAMPLER,0},
	});
	st.fxaaPass->prepare();
}

void recordCommand() {
	st.mipmapCommand = new uint32_t[st.cfg.vkcfgMaxImagesInFlight];
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.rd.drAllocateCommandBuffer(&st.mipmapCommand[i]);
		st.rd.drStartCommandRecording(st.mipmapCommand[i]);
		st.rd.drSetImageLayoutSimple(st.canvas, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, st.mipmapCommand[i]);
		st.canvas->registerMipmapGenCommand(st.mipmapCommand[i]);
		st.rd.drEndCommandRecording(st.mipmapCommand[i]);
	}

	// Register commands
	st.mainPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.model.getVertexBuffer(), x);
		st.rd.drBindIndexBuffer(st.model.getIndexBuffer(), x);
		st.rd.drDrawIndexedIndirect(st.model.getIndirectBuffer(), x);
	});
	st.outlinePass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.model.getVertexBuffer(), x);
		st.rd.drBindIndexBuffer(st.model.getIndexBuffer(), x);
		st.rd.drDrawIndexedIndirect(st.model.getIndirectBuffer(), x);
	});
	st.fxaaPass->recordCommand();

	st.passSeq[0] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.passSeq[1] = std::make_unique<AnthemSequentialCommand>(&st.rd);

	st.passSeq[0]->setSequence({
		{ st.mainPass->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.outlinePass->getCommandIndex(0), ATC_ASCE_GRAPHICS},
		{ st.mipmapCommand[0], ATC_ASCE_GRAPHICS},
		{ st.fxaaPass->getCommandIdx(0), ATC_ASCE_GRAPHICS},
	});
	st.passSeq[1]->setSequence({
		{ st.mainPass->getCommandIndex(1), ATC_ASCE_GRAPHICS} ,
		{ st.outlinePass->getCommandIndex(1), ATC_ASCE_GRAPHICS},
		{ st.mipmapCommand[1], ATC_ASCE_GRAPHICS},
		{ st.fxaaPass->getCommandIdx(1), ATC_ASCE_GRAPHICS},
	});
}

void updateUniform() {
	AtMatf4 proj, view, local;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	st.orbitControl.getModelMatrix(local);

	float pm[16], vm[16], lm[16], vc[4];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);
	vc[0] = st.aspect;
	st.uniform->specifyUniforms(pm, vm, lm, vc);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniform->updateBuffer(i);
	}
}

void drawCall() {
	static int cur = 0;
	updateUniform();
	uint32_t imgIdx = 0;
	st.rd.drPrepareFrame(cur, &imgIdx);
	prepareImguiFrame();
	st.passSeq[cur]->executeCommandToStage(imgIdx, false,true, st.fxaaPass->getSwapchainFb());
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}


int main() {
	initialize();
	setupImgui();
	loadModel();
	prepareMainPass();
	prepareOutlinePass();
	preparePostAA();

	st.rd.registerPipelineSubComponents();
	recordCommand();

	st.rd.setDrawFunction(drawCall);
	st.rd.startDrawLoopDemo();
	st.rd.exDestroyImgui();
	st.rd.finalize();
	
	return 0;
}