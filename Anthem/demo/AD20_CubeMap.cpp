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

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Camera;
using namespace Anthem::External;
using namespace Anthem::Core;


#define ATRANGE(x,y) (std::views::iota((x),(y)))

struct Stage {
	// Core
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemFrameRateMeter fpsmeter = AnthemFrameRateMeter(10);

	// Geometry
	AnthemVertexBufferImpl<AtAttributeVecf<4>>* sbox;
	AnthemIndexBuffer* ix;

	// Texture & Viewport
	AnthemDescriptorPool* desc;
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uni;
	AnthemImageCubic* tex;

	// Display
	AnthemRenderPass* pass;
	AnthemGraphicsPipeline* pipe;
	AnthemSwapchainFramebuffer* fb;
	AnthemDepthBuffer* depth;
	AnthemShaderFilePaths path;
	AnthemShaderModule* shader;

	AnthemRenderPassSetupOption ropt;
	AnthemGraphicsPipelineCreateProps copt;

	AnthemSemaphore* semaphore;

	// Const
	constexpr static const int inFlight = 2;
}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "texcube\\texcube.";
	st += x;
	st += ".hlsl.spv";
	return st;
}

inline std::string getSkyboxTex(auto x) {
	std::string st(ANTH_ASSET_DIR);
	st += "skybox\\";
	st += x;
	st += ".jpg";
	return st;
}

void initialize() {
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.camera.specifyFrustum((float)AT_PI / 3.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 0, 0);
	st.camera.specifyFrontEyeRay(0, 0, 1);
}

void createGeometry() {
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
	for (auto i : ATRANGE(0, 8))st.sbox->insertData(i, vertices[i]);
}

void createTextures() {
	const char* fileNames[6] = { "right","left","top","bottom","front","back" };
	std::array<uint8_t*, 6> rawData;
	uint32_t width, height, channel;
	AnthemImageLoader loader;
	for (auto i : ATRANGE(0, 6)) {
		loader.loadImage(getSkyboxTex(fileNames[i]).c_str(), &width, &height, &channel, &rawData[i]);
	}
	st.rd.createDescriptorPool(&st.desc);
	st.rd.createCubicTextureSimple(&st.tex, st.desc, rawData, width, height, channel, 0, -1);
}

void createUniform() {
	st.rd.createUniformBuffer(&st.uni, 0, st.desc, -1);
}

void updateUniform() {
	AtMatf4 proj, view, model;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	model = AnthemLinAlg::axisAngleRotationTransform3<float,float>({ 0.0f,1.0f,0.0f }, 0.1f * static_cast<float>(glfwGetTime()));


	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	model.columnMajorVectorization(lm);

	st.uni->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.inFlight; i++) {
		st.uni->updateBuffer(i);
	}
}

void prepareStage() {
	st.path.vertexShader = getShader("vert");
	st.path.fragmentShader = getShader("frag");
	st.rd.createShader(&st.shader, &st.path);
	st.rd.createDepthBuffer(&st.depth, false);

	st.ropt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	st.ropt.msaaType = AT_ARPMT_NO_MSAA;
	st.rd.setupRenderPass(&st.pass, &st.ropt, st.depth);
	st.rd.createSwapchainImageFramebuffers(&st.fb, st.pass, st.depth);

	AnthemDescriptorSetEntry dseUniform{ st.desc,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseSampler{ st.desc,AT_ACDS_SAMPLER,0 };
	st.copt.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
	st.rd.createGraphicsPipelineCustomized(&st.pipe, { dseUniform,dseSampler }, {}, st.pass, st.shader, st.sbox, &st.copt);

	st.rd.createSemaphore(&st.semaphore);
}

void recordCommand() {
	auto& r = st.rd;
	for (uint32_t i : ATRANGE(0, st.inFlight)) {
		r.drStartCommandRecording(i);
		r.drStartRenderPass(st.pass, st.fb->getFramebufferObjectUnsafe(i), i, false);
		r.drSetViewportScissorFromSwapchain(i);
		r.drBindVertexBuffer(st.sbox,i);
		r.drBindIndexBuffer(st.ix, i);
		r.drBindGraphicsPipeline(st.pipe, i);
		AnthemDescriptorSetEntry dseUniform{ st.desc,AT_ACDS_UNIFORM_BUFFER,i };
		AnthemDescriptorSetEntry dseSampler{ st.desc,AT_ACDS_SAMPLER,i };
		r.drBindDescriptorSetCustomizedGraphics({ dseUniform,dseSampler }, st.pipe, i);
		r.drDraw(st.ix->getIndexCount(), i);
		r.drEndRenderPass(i);
		r.drEndCommandRecording(i);
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
	st.fpsmeter.record();
	std::stringstream ss;
	ss << "FPS:";
	ss << st.fpsmeter.getFrameRate();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Control Panel");
	ImGui::Text(ss.str().c_str());
	ImGui::End();
}

void mainLoop() {
	prepareImguiFrame();
	updateUniform();
	static int cur = 0;
	std::vector<const AnthemSemaphore*> waitImgui = {st.semaphore};
	std::vector<AtSyncSemaphoreWaitStage> waitImguiStage = { AtSyncSemaphoreWaitStage::AT_SSW_COLOR_ATTACH_OUTPUT };
	std::vector<const AnthemSemaphore*> waitDisplay = {};
	std::vector<AtSyncSemaphoreWaitStage> waitStageDisplay = {  };
	std::vector<const AnthemSemaphore*> dispSemaphoreSignal = {st.semaphore};
	uint32_t imgIdx;
	st.rd.drPrepareFrame(cur, &imgIdx);
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	st.rd.exRenderImGui(cur, st.fb, { 0,0,0,1 }, drawData);
	uint32_t imguiCmdBuf;
	st.rd.exGetImGuiCommandBufferIndex(cur, &imguiCmdBuf);
	AnthemSemaphore* imguiSemaphore;
	st.rd.exGetImGuiDrawProgressSemaphore(cur, &imguiSemaphore);
	std::vector<const AnthemSemaphore*> imguiDone = { imguiSemaphore };


	st.rd.drSubmitCommandBufferGraphicsQueueGeneral2(cur, imgIdx, &waitDisplay, &waitStageDisplay, nullptr, &dispSemaphoreSignal);
	st.rd.drSubmitCommandBufferGraphicsQueueGeneral(imguiCmdBuf, imgIdx, &waitImgui, &waitImguiStage, nullptr, false);
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}



int main() {
	initialize();
	setupImgui();
	createTextures();
	createGeometry();
	createUniform();
	prepareStage();
	st.rd.registerPipelineSubComponents();

	recordCommand();
	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.exDestroyImgui();
	st.rd.finalize();
	return 0;
}