#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/drawing/buffer/AnthemShaderStorageBuffer.h"
#include "../include/core/drawing/buffer/AnthemUniformBuffer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/components/camera/AnthemCamera.h"

#include <chrono>

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::Components::Camera;

using namespace std::chrono;


struct ExpParams {
	// Exp params
	static AtVecf4 centerTranslation;
	static AtVecf4 rotationAxis;

	// Parallel configurations
	static constexpr int sampleCounts = 1048576;
	static constexpr int parallelsX = 8192;

	// Visualization
	static constexpr float lineWidth = 2;
};

struct ExpCore {
	AnthemSimpleToyRenderer renderer;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
}core;

struct ComputePipeline {
	AnthemShaderStorageBufferImpl<AtBufVecd4f<1>, AtBufVecd4f<1>>* samples = nullptr;
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
	AnthemSemaphore** firstStageDone = nullptr;
}vis;

struct CoordAxisVisPipeline {
	AnthemVertexBufferImpl<AnthemVAOAttrDesc<float, 4>, AnthemVAOAttrDesc<float, 4>>* vxBuf;
	AnthemIndexBuffer* ixBuf;
	AnthemShaderFilePaths shaderFile;
	AnthemShaderModule* shader = nullptr;
	AnthemGraphicsPipeline* pipeline = nullptr;
	AnthemRenderPass* renderPass = nullptr;

	uint32_t* axisCmdBuf = nullptr;
	AnthemFence** drawProgress = nullptr;
	AnthemSemaphore** drawAvailable = nullptr;
	AnthemGraphicsPipelineCreateProps cprop;
}axis;

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
			w->setInput(i, { fx, fy, 1.0f, 0.0f }, { fx, fy, 1.0f, 0.0f });
		}
	};
	core.renderer.createShaderStorageBuffer(&comp.samples, ExpParams::sampleCounts, 0, comp.descPoolSsbo, std::make_optional(ssboCreateFunc));

	// Create Uniform Buffer
	core.renderer.createUniformBuffer(&comp.uniform, 0, comp.descPoolUniform);
	float temp1[4] = { 0,0,5,0 };
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

	// Create Sync
	vis.firstStageDone = new AnthemSemaphore * [core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.createSemaphore(&vis.firstStageDone[i]);
	}
}

void prepareAxisVis() {
	// Create req bufs
	core.renderer.createVertexBuffer(&axis.vxBuf);
	axis.vxBuf->setTotalVertices(2);
	axis.vxBuf->insertData(0, { -0.5f, 0.5f, 0.5f,1.0f }, { -0.5f, 0.5f, 0.5f,1.0f });
	axis.vxBuf->insertData(1, { -0.5f, -0.5f, 0.5f,1.0f }, { -0.5f, -0.5f, 0.5f,1.0f });

	core.renderer.createIndexBuffer(&axis.ixBuf);
	axis.ixBuf->setIndices({ 0, 1 });

	// Load shaders
	axis.shaderFile.vertexShader = getShader("axis.vert");
	axis.shaderFile.fragmentShader = getShader("axis.frag");
	core.renderer.createShader(&axis.shader, &axis.shaderFile);

	core.renderer.setupDemoRenderPass(&axis.renderPass, vis.depthBuffer,true);

	axis.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_LINE;
	std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { };
	core.renderer.createGraphicsPipelineCustomized(&axis.pipeline, descSetEntriesRegPipeline, axis.renderPass, axis.shader,
		axis.vxBuf, &axis.cprop);

	axis.drawProgress = new AnthemFence * [core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	axis.drawAvailable = new AnthemSemaphore* [core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.createFence(&axis.drawProgress[i]);
		core.renderer.createSemaphore(&axis.drawAvailable[i]);
	}

	// Allocate Command Buffers
	axis.axisCmdBuf = new uint32_t[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.drAllocateCommandBuffer(&axis.axisCmdBuf[i]);
	}
}

void recordCommandBufferDrwAxis(int i) {
	auto& renderer = core.renderer;
	renderer.drStartRenderPass(axis.renderPass, (AnthemFramebuffer*)(vis.framebuffer->getFramebufferObject(i)), axis.axisCmdBuf[i], false);
	renderer.drSetViewportScissor(axis.axisCmdBuf[i]);
	renderer.drBindGraphicsPipeline(axis.pipeline, axis.axisCmdBuf[i]);
	renderer.drBindVertexBuffer(axis.vxBuf, axis.axisCmdBuf[i]);
	renderer.drBindIndexBuffer(axis.ixBuf, axis.axisCmdBuf[i]);
	renderer.drSetLineWidth(ExpParams::lineWidth, axis.axisCmdBuf[i]);
	renderer.drDraw(axis.ixBuf->getIndexCount(), axis.axisCmdBuf[i]);
	renderer.drEndRenderPass(axis.axisCmdBuf[i]);
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
		.inTypeIndex = 1
	};
	AnthemDescriptorSetEntry dseSsboOut = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
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
		renderer.drStartCommandRecording(axis.axisCmdBuf[i]);
		recordCommandBufferDrwAxis(i);
		renderer.drEndCommandRecording(axis.axisCmdBuf[i]);
	}
}

void drawLoop(int& currentFrame) {
	
	uint32_t imgIdx;
	
	core.renderer.drPrepareFrame(currentFrame, &imgIdx);

	// Compute Task
	comp.computeProgress[0]->waitForFence();
	comp.computeProgress[0]->resetFence();

	std::vector<const AnthemSemaphore*> semaphoreToSignal = { comp.computeDone[0] };
	std::vector<AtSyncSemaphoreWaitStage> waitStage = { AtSyncSemaphoreWaitStage::AT_SSW_VERTEX_INPUT };
	std::vector<const AnthemSemaphore*> semaphoreToSignalAxis = { axis.drawAvailable[currentFrame] };
	std::vector<AtSyncSemaphoreWaitStage> waitStageAxis = { AtSyncSemaphoreWaitStage::AT_SSW_COLOR_ATTACH_OUTPUT };

	core.renderer.drSubmitCommandBufferCompQueueGeneral(comp.computeCmdBufIdx[0], nullptr, &semaphoreToSignal, comp.computeProgress[0]);

	// Graphics Drawing
	std::vector<const AnthemSemaphore*> firstStageDone = { vis.firstStageDone[currentFrame]};
	ANTH_ASSERT((vis.firstStageDone[currentFrame] != nullptr),"No");
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral2(currentFrame, imgIdx, &semaphoreToSignal, &waitStage,nullptr,&firstStageDone);
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral(axis.axisCmdBuf[currentFrame], imgIdx, &firstStageDone,
		&waitStageAxis);

	core.renderer.drPresentFrame(currentFrame, imgIdx);
	currentFrame++;
	currentFrame %= 2;
}


int main() {
	prepareCore();
	std::once_flag p1;
	auto startGenTime = std::chrono::steady_clock().now();
	prepareComputePipeline();
	prepareVisualization();
	prepareAxisVis();
	
	core.renderer.registerPipelineSubComponents();
	recordCommandBufferAll();
	recordCommandBufferCompAll();

	int currentFrame = 0;
	auto startTime = std::chrono::steady_clock().now();

	core.renderer.setDrawFunction([&]() {
		drawLoop(currentFrame);
		std::call_once(p1, [&]() {
			//comp.computeProgress[0]->waitForFence();
			//comp.computeProgress[0]->resetFence();

			auto endTime = std::chrono::steady_clock().now();
			auto durationGen = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startGenTime);
			auto durationCalc = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
			ANTH_LOGI("Time elapsed:");
			ANTH_LOGI("w/o Init:", durationCalc.count()," ms");
			ANTH_LOGI("w/ Init:", durationGen.count(), " ms");
			ANTH_LOGI("");
		});
	});
	core.renderer.startDrawLoopDemo();
	core.renderer.finalize();

	return 0;
}