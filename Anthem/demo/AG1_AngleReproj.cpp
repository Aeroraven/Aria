#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/drawing/buffer/AnthemShaderStorageBuffer.h"
#include "../include/core/drawing/buffer/AnthemUniformBuffer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/components/camera/AnthemCamera.h"

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::Components::Camera;


struct ExpParams {
	// Exp params
	static AtVecf4 centerTranslation;
	static AtVecf4 rotationAxis;

	// Running configurations
	static constexpr int sampleCounts = 4096;
	static constexpr int parallelsX = 256;
};

struct ExpCore {
	AnthemSimpleToyRenderer renderer;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
}core;

struct ComputePipeline {
	AnthemShaderStorageBufferImpl<AtBufVecd4f<1>>* samples = nullptr;
	AnthemUniformBufferImpl<AnthemUniformVecf<4>, AnthemUniformVecf<4>>* uniform = nullptr;

	AnthemDescriptorPool* descPoolUniform = nullptr;
	AnthemDescriptorPool* descPoolSsbo = nullptr;

	AnthemShaderFilePaths shaderFile;
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
}vis;


inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "angleReproj\\shader.";
	st += x;
	st += ".spv";
	return st;
}

void prepareCore() {
	core.renderer.setConfig(&core.cfg);
	core.renderer.initialize();
}

void prepareComputePipeline() {
	// Create descriptors
	core.renderer.createDescriptorPool(&comp.descPoolSsbo);
	core.renderer.createDescriptorPool(&comp.descPoolUniform);

	// Loading shader
	comp.shaderFile.computeShader = getShader("comp");
	core.renderer.createShader(&comp.shader, &comp.shaderFile);

	// Filling experiment data
	using ssboType = std::remove_cv_t<decltype(comp.samples)>;
	std::function<void(ssboType)> ssboCreateFunc = [&](ssboType w) {
		for (auto i : std::ranges::views::iota(0,ExpParams::sampleCounts)) {
			auto fx = AnthemLinAlg::randomNumber<float>();
			auto fy = AnthemLinAlg::randomNumber<float>();
			fx = 2.0f * (fx - 0.5f);
			fy = 2.0f * (fy - 0.5f);
			w->setInput(i, { fx, fy,0.0f,0.0f });
		}
		};
	core.renderer.createShaderStorageBuffer(&comp.samples, ExpParams::sampleCounts, 0, comp.descPoolSsbo, std::make_optional(ssboCreateFunc));

	// Create Uniform Buffer
	core.renderer.createUniformBuffer(&comp.uniform, 0, comp.descPoolUniform);
	float temp1[4] = { 0,0,0,0 };
	float temp2[4] = { 0,0,0,0 };
	comp.uniform->specifyUniforms(temp1, temp2);


	// Create Pipeline
	AnthemDescriptorSetEntry dseUniformIn = {
		.descPool = comp.descPoolUniform,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboIn = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboOut = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> vct = { dseUniformIn,dseSsboIn,dseSsboOut };
	core.renderer.createComputePipelineCustomized(&comp.pipeline, vct, comp.shader);

	// Allocate Command Buffers
	comp.computeCmdBufIdx = new uint32_t[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0,core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.drAllocateCommandBuffer(&comp.computeCmdBufIdx[i]);
	}

	// Create Sync Objects
	comp.computeProgress = new AnthemFence*[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	comp.computeDone = new AnthemSemaphore*[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0,core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.createFence(&comp.computeProgress[i]);
		core.renderer.createSemaphore(&comp.computeDone[i]);
	}
}

void prepareVisualization() {
	// Create required buffers
	core.renderer.createIndexBuffer(&vis.ix);
	core.renderer.createDepthBuffer(&vis.depthBuffer, false);
	auto ind = std::ranges::views::iota(0,ExpParams::sampleCounts) | std::ranges::to<std::vector<unsigned int>>();
	vis.ix->setIndices(ind);

	// Load shaders
	vis.shaderFile.vertexShader = getShader("vert");
	vis.shaderFile.fragmentShader = getShader("frag");
	core.renderer.createShader(&vis.shader, &vis.shaderFile);

	// Create Pipeline
	core.renderer.setupDemoRenderPass(&vis.renderPass, vis.depthBuffer);
	core.renderer.createSwapchainImageFramebuffers(&vis.framebuffer, vis.renderPass, vis.depthBuffer);
	
	vis.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_POINT_LIST;

	std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { };
	core.renderer.createGraphicsPipelineCustomized(&vis.pipeline, descSetEntriesRegPipeline, vis.renderPass, vis.shader,
		comp.samples, &vis.cprop);
}

void recordCommandBufferDrw(int i) {
	auto& renderer = core.renderer;
	renderer.drStartRenderPass(vis.renderPass, (AnthemFramebuffer*)(vis.framebuffer->getFramebufferObject(i)), i, false);
	renderer.drSetViewportScissor(i);
	renderer.drBindGraphicsPipeline(vis.pipeline, i);
	renderer.drBindVertexBufferFromSsbo(comp.samples, 0, i);
	renderer.drBindIndexBuffer(vis.ix, i);

	renderer.drDraw(vis.ix->getIndexCount(), i);
	renderer.drEndRenderPass(i);
}


void recordCommandBufferComp(int i) {
	auto& renderer = core.renderer;
	renderer.drBindComputePipeline(comp.pipeline, comp.computeCmdBufIdx[i]);
	AnthemDescriptorSetEntry dseUniform = {
		.descPool = comp.descPoolUniform,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboIn = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = static_cast<uint32_t>(i)
	};
	AnthemDescriptorSetEntry dseSsboOut = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = (static_cast<uint32_t>(i) + 1) % 2
	};
	std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { dseUniform,dseSsboIn,dseSsboOut };
	renderer.drBindDescriptorSetCustomizedCompute(descSetEntriesRegPipeline, comp.pipeline, comp.computeCmdBufIdx[i]);
	renderer.drComputeDispatch(comp.computeCmdBufIdx[i], ExpParams::parallelsX, 1, 1);

}


void recordCommandBufferCompAll() {
	auto& renderer = core.renderer;
	for (int i = 0; i < core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
		renderer.drStartCommandRecording(comp.computeCmdBufIdx[i]);
		ANTH_LOGI("Start Recording:", comp.computeCmdBufIdx[i]);
		recordCommandBufferComp(i);
		renderer.drEndCommandRecording(comp.computeCmdBufIdx[i]);
	}
}

void recordCommandBufferAll() {
	auto& renderer = core.renderer;
	for (int i = 0; i < core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
		renderer.drStartCommandRecording(i);
		recordCommandBufferDrw(i);
		renderer.drEndCommandRecording(i);
	}
}

void drawLoop(int& currentFrame) {

	uint32_t imgIdx;
	core.renderer.drPrepareFrame(currentFrame, &imgIdx);

	// Compute Task
	comp.computeProgress[currentFrame]->waitForFence();
	comp.computeProgress[currentFrame]->resetFence();

	std::vector<const AnthemSemaphore*> semaphoreToSignal = { comp.computeDone[currentFrame] };
	std::vector<AtSyncSemaphoreWaitStage> waitStage = { AtSyncSemaphoreWaitStage::AT_SSW_VERTEX_INPUT };
	core.renderer.drSubmitCommandBufferCompQueueGeneral(comp.computeCmdBufIdx[currentFrame], nullptr, &semaphoreToSignal, comp.computeProgress[currentFrame]);

	// Graphics Drawing
	//shared.renderer.drSubmitBufferPrimaryCall(currentFrame, currentFrame);
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral(currentFrame, imgIdx, &semaphoreToSignal, &waitStage);
	core.renderer.drPresentFrame(currentFrame, imgIdx);
	currentFrame++;
	currentFrame %= 2;
}


int main() {
	prepareCore();
	prepareComputePipeline();
	prepareVisualization();

	core.renderer.registerPipelineSubComponents();
	recordCommandBufferAll();
	recordCommandBufferCompAll();

	int currentFrame = 0;
	core.renderer.setDrawFunction([&]() {
		drawLoop(currentFrame);
		});
	core.renderer.startDrawLoopDemo();
	core.renderer.finalize();

	return 0;
}