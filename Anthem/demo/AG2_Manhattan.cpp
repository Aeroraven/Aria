#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/drawing/buffer/AnthemShaderStorageBuffer.h"
#include "../include/core/drawing/buffer/AnthemUniformBuffer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/components/camera/AnthemCamera.h"

#include <chrono>
#include <thread>
#include <mutex>

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::Components::Camera;
using namespace std::chrono;

struct ExpCore {
	AnthemSimpleToyRenderer renderer;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
}core;

struct ComputePipeline {
	AnthemShaderStorageBufferImpl<
		AtBufVecd4f<1>, //Rot
		AtBufVecd4f<1>  //Trans
	>* ssbo = nullptr;
	AnthemUniformBufferImpl <
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>
	>* uniform = nullptr;

	AnthemDescriptorPool* descPool = nullptr;
	AnthemDescriptorPool* descPoolUni = nullptr;
	AnthemShaderFilePaths shaderPath;
	AnthemShaderModule* shader = nullptr;
	AnthemComputePipeline* pipeline = nullptr;
	AnthemFence** computeProgress = nullptr;;
	AnthemSemaphore** computeDone = nullptr;
	uint32_t* computeCmdBufIdx = nullptr;
}comp;

struct VisualizationPipeline {
	AnthemIndexBuffer* ix = nullptr;
	AnthemDepthBuffer* depthBuffer = nullptr;

	AnthemShaderFilePaths shaderFile;
	AnthemShaderModule* shader = nullptr;
	AnthemGraphicsPipeline* pipeline = nullptr;
	AnthemSwapchainFramebuffer* framebuffer = nullptr;
	AnthemRenderPass* renderPass = nullptr;
	AnthemGraphicsPipelineCreateProps cprop;

	using uProjMat = AnthemUniformMatf<4>;
	using uViewMat = AnthemUniformMatf<4>;
	using uLocalMat = AnthemUniformMatf<4>;
	AnthemDescriptorPool* descVisUniform = nullptr;
	AnthemUniformBufferImpl<uProjMat, uViewMat, uLocalMat>* visUniform = nullptr;
}vis;

struct ExpParams {
	static constexpr const int sampleCounts = 65536*16; //Total samples
	static constexpr const int parallelsGpu = 65536; // For GPU kernels 16384
	static constexpr const int parallelsCpu = 16; // For CPU threads

	float rotXLower = -89.0f;
	float rotXUpper = 89.0f;
	float rotYLower = -0.0f;
	float rotYUpper = 0.0f;

	float transDeviation = 25.0f;
	float transZLower = 5.0f;
	float transZUpper = 45.0f;

	float rectW = 1.0f;
	float rectH = 1.0f;

	float visRot = 0.0f;

}expParam;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "manhattan\\shader.";
	st += x;
	st += ".spv";
	return st;
}

void prepareCore() {
	core.cfg.APP_NAME = "Anthem [AG2_Manhattan]";
	core.renderer.setConfig(&core.cfg);
	core.renderer.initialize();
}

void setupComputePipeline() {
	// Create descriptors
	core.renderer.createDescriptorPool(&comp.descPool);
	core.renderer.createDescriptorPool(&comp.descPoolUni);

	// Loading shader
	comp.shaderPath.computeShader = getShader("comp");
	core.renderer.createShader(&comp.shader, &comp.shaderPath);

	// Create samples
	using ssboType = std::remove_cv_t<decltype(comp.ssbo)>;
	std::function<void(ssboType)> ssboCreateFunc = [&](ssboType w) {
		auto childJob = [&](int segment, int totalSegments)->void {
			auto st = ExpParams::sampleCounts / totalSegments * segment;
			auto ed = ExpParams::sampleCounts / totalSegments * (segment + 1);
			for (auto i : std::ranges::views::iota(st, ed)) {
				auto rotDeg = AnthemLinAlg::randomNumber<float>();
				auto rotDir = (AnthemLinAlg::randomNumber<float>() );
				auto transDeg = AnthemLinAlg::randomNumber<float>();
				auto transLen = AnthemLinAlg::randomNumber<float>();
				auto transZ = AnthemLinAlg::randomNumber<float>();

				w->setInput(i, { rotDeg,rotDir,0.0f,0.0f }, { transDeg,transLen,transZ,0.0f });
			}
		};
		std::vector<std::thread> jobs;
		for (auto i : std::views::iota(0, ExpParams::parallelsCpu)) {
			std::thread childExec(childJob, i, ExpParams::parallelsCpu);
			jobs.push_back(std::move(childExec));
		}
		for (auto& p : jobs) {
			p.join();
		}
		ANTH_LOGI("Random numbers generated");
	};
	core.renderer.createShaderStorageBuffer(&comp.ssbo, ExpParams::sampleCounts, 0, comp.descPool, std::make_optional(ssboCreateFunc));
	core.renderer.createUniformBuffer(&comp.uniform, 0, comp.descPoolUni);
	comp.uniform->specifyUniforms(
		&expParam.rotXLower,
		&expParam.rotXUpper,
		&expParam.rotYLower,
		&expParam.rotYUpper,
		&expParam.transDeviation,
		&expParam.transZLower,
		&expParam.transZUpper,
		&expParam.rectH,
		&expParam.rectW
	);
	for (int i : std::views::iota(0,core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		comp.uniform->updateBuffer(i);
	}

	// Create Pipeline
	AnthemDescriptorSetEntry dseUniformIn = {
		.descPool = comp.descPoolUni,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboIn = {
		.descPool = comp.descPool,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboOut = {
		.descPool = comp.descPool,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> vct = { dseUniformIn,dseSsboIn,dseSsboOut };
	core.renderer.createComputePipelineCustomized(&comp.pipeline, vct, comp.shader);

	// Allocate Command Buffers
	comp.computeCmdBufIdx = new uint32_t[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.drAllocateCommandBuffer(&comp.computeCmdBufIdx[i]);
	}

	// Create Sync Objects
	comp.computeProgress = new AnthemFence * [core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	comp.computeDone = new AnthemSemaphore * [core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.createFence(&comp.computeProgress[i]);
		core.renderer.createSemaphore(&comp.computeDone[i]);
	}
}

void updateVisUniform() {
	int rdH, rdW;
	core.renderer.exGetWindowSize(rdH, rdW);
	core.camera.specifyFrustum(AT_PI / 3 * 1, 0.1, 100, 1.0 * rdW / rdH);
	core.camera.specifyPosition(0.0, 0.0, -3.4f);
	core.camera.specifyFrontEyeRay(0, -0.0, 3.4f);

	auto axis = Math::AnthemVector<float, 3>({ 0.0f,1.0f,0.0f });
	auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis, expParam.visRot);
	AtMatf4 proj, view, model;
	AtMatf4 eye = Math::AnthemLinAlg::eye<float, 4>();

	float projRaw[16], viewRaw[16], modelRaw[16], eyeRaw[16];
	core.camera.getProjectionMatrix(proj);
	core.camera.getViewMatrix(view);
	model = local.multiply(Math::AnthemLinAlg::eye<float, 4>());

	proj.columnMajorVectorization(projRaw);
	view.columnMajorVectorization(viewRaw);
	model.columnMajorVectorization(modelRaw);
	eye.columnMajorVectorization(eyeRaw);

	AtMatf4 eyeProj = Math::AnthemLinAlg::eye<float, 4>();
	float eyeProjRaw[16];
	eyeProj[0][0] = 0.3;
	eyeProj[1][1] = -0.3 * rdW / rdH;
	eyeProj.columnMajorVectorization(eyeProjRaw);
	vis.visUniform->specifyUniforms(projRaw, viewRaw, modelRaw);
	for (auto i : std::ranges::views::iota(0, 2)) {
		vis.visUniform->updateBuffer(i);
	}

}

void prepareVisualization() {
	// Create required buffers
	core.renderer.createIndexBuffer(&vis.ix);
	core.renderer.createDepthBuffer(&vis.depthBuffer, false);

	vis.shaderFile.vertexShader = getShader("vert");
	vis.shaderFile.fragmentShader = getShader("frag");
	//vis.shaderFile.geometryShader = getShader("geom");
	core.renderer.createShader(&vis.shader, &vis.shaderFile);

	auto indPt = std::ranges::views::iota(0, ExpParams::sampleCounts) | 
		std::ranges::to<std::vector<unsigned int>>();
	vis.ix->setIndices(indPt);

	// Create Uniform Buffer
	core.renderer.createDescriptorPool(&vis.descVisUniform);
	core.renderer.createUniformBuffer(&vis.visUniform, 0, vis.descVisUniform);
	updateVisUniform();


	// Create Pipeline
	AnthemRenderPassSetupOption opt;
	opt.predefinedClearColor = { 1,1,1,1.0 };
	opt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	opt.msaaType = AT_ARPMT_NO_MSAA;

	core.renderer.setupRenderPass(&vis.renderPass, &opt, vis.depthBuffer);
	core.renderer.createSwapchainImageFramebuffers(&vis.framebuffer, vis.renderPass, vis.depthBuffer);

	AnthemDescriptorSetEntry dseUniform = {
		.descPool = vis.descVisUniform,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};

	std::vector<AnthemDescriptorSetEntry> vct = { dseUniform  };
	vis.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_POINT_LIST;
	core.renderer.createGraphicsPipelineCustomized(&vis.pipeline, vct, {}, vis.renderPass, vis.shader, comp.ssbo, &vis.cprop);
}

void recordCommandBufferComp(int i) {
	auto& renderer = core.renderer;
	renderer.drBindComputePipeline(comp.pipeline, comp.computeCmdBufIdx[i]);
	AnthemDescriptorSetEntry dseUniform = {
		.descPool = comp.descPoolUni,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboIn = {
		.descPool = comp.descPool,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 1
	};
	AnthemDescriptorSetEntry dseSsboOut = {
		.descPool = comp.descPool,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { dseUniform,dseSsboIn,dseSsboOut };
	renderer.drBindDescriptorSetCustomizedCompute(descSetEntriesRegPipeline, comp.pipeline, comp.computeCmdBufIdx[i]);
	renderer.drComputeDispatch(comp.computeCmdBufIdx[i], ExpParams::parallelsGpu, 1, 1);
}

void recordCommandBufferDrw(int i) {
	auto& renderer = core.renderer;
	renderer.drStartRenderPass(vis.renderPass, (AnthemFramebuffer*)(vis.framebuffer->getFramebufferObject(i)), i, false);
	renderer.drSetViewportScissorFromSwapchain(i);
	renderer.drBindGraphicsPipeline(vis.pipeline, i);
	renderer.drBindVertexBufferFromSsbo(comp.ssbo, 0, i);
	renderer.drBindIndexBuffer(vis.ix, i);

	AnthemDescriptorSetEntry dseUniform = {
		.descPool = vis.descVisUniform,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	renderer.drSetLineWidth(1, i);
	std::vector<AnthemDescriptorSetEntry> descSetEntries = { dseUniform };
	renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries, vis.pipeline, i);

	renderer.drDraw(vis.ix->getIndexCount(), i);
	renderer.drEndRenderPass(i);
}

void recordCommandBufferAll() {
	auto& renderer = core.renderer;
	for (int i = 0; i < core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
		renderer.drStartCommandRecording(comp.computeCmdBufIdx[i]);
		ANTH_LOGI("Start Recording:", comp.computeCmdBufIdx[i]);
		recordCommandBufferComp(i);
		renderer.drEndCommandRecording(comp.computeCmdBufIdx[i]);

		renderer.drStartCommandRecording(i);
		recordCommandBufferDrw(i);
		renderer.drEndCommandRecording(i);
	}
}

void drawLoop(int& currentFrame) {
	uint32_t imgIdx;
	core.renderer.drPrepareFrame(currentFrame, &imgIdx);
	// Computing
	comp.computeProgress[0]->waitForFence();
	comp.computeProgress[0]->resetFence();

	std::vector<const AnthemSemaphore*> semaphoreToSignal = { comp.computeDone[0] };
	std::vector<AtSyncSemaphoreWaitStage> waitStage = { AtSyncSemaphoreWaitStage::AT_SSW_VERTEX_INPUT };
	core.renderer.drSubmitCommandBufferCompQueueGeneral(comp.computeCmdBufIdx[0], nullptr, &semaphoreToSignal, comp.computeProgress[0]);
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral(currentFrame, imgIdx, &semaphoreToSignal,&waitStage);
	core.renderer.drPresentFrame(currentFrame, imgIdx);
	currentFrame++;
	currentFrame %= 2;
}


int main() {
	prepareCore();
	setupComputePipeline();
	prepareVisualization();
	core.renderer.registerPipelineSubComponents();
	recordCommandBufferAll();

	int currentFrame = 0;
	auto startTime = std::chrono::steady_clock().now();

	core.renderer.setDrawFunction([&]() {
		expParam.visRot += 0.005;
		updateVisUniform();
		drawLoop(currentFrame);
	});
	core.renderer.startDrawLoopDemo();
	core.renderer.finalize();

	return 0;
}