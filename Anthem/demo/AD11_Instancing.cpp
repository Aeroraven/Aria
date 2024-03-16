#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"


using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::External;
using namespace Anthem::Components::Camera;

struct BaseComponents {
	AnthemSimpleToyRenderer renderer;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
}core;

struct MainPipeline {
	AnthemVertexBufferImpl<AnthemVAOAttrDesc<float, 2>>* vvbuf;
	AnthemInstancingVertexBufferImpl<AnthemVAOAttrDesc<float, 2>>* vibuf;

	AnthemIndexBuffer* ibuf;

	AnthemShaderFilePaths shaderFile;
	AnthemShaderModule* shader = nullptr;

	AnthemRenderPass* renderPass = nullptr;
	AnthemGraphicsPipeline* pipeline = nullptr;
	AnthemDepthBuffer* depthBuffer = nullptr;
	AnthemSwapchainFramebuffer* framebuffer = nullptr;
	AnthemGraphicsPipelineCreateProps cprop;

	constexpr static int instances = 3;
}drw;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "instancing\\shader.";
	st += x;
	st += ".spv";
	return st;
}

void prepareBaseComponents(){
	core.renderer.setConfig(&core.cfg);
	core.renderer.initialize();
}

void buildPipeline() {
	core.renderer.createVertexBuffer(&drw.vvbuf);
	drw.vvbuf->setTotalVertices(3);
	drw.vvbuf->insertData(0, { -0.2,0.0 });
	drw.vvbuf->insertData(1, { 0.2,0.0 });
	drw.vvbuf->insertData(2, { 0.0,0.2 });

	core.renderer.createIndexBuffer(&drw.ibuf);
	drw.ibuf->setIndices({ 0,1,2 });

	core.renderer.createInstancingBuffer(&drw.vibuf);
	drw.vibuf->setTotalVertices(MainPipeline::instances);
	drw.vibuf->insertData(0, { -0.5,0.0 });
	drw.vibuf->insertData(1, { 0.5,0.0 });
	drw.vibuf->insertData(2, { 0,0.5 });
	drw.vibuf->setAttrBindingPoint({ 1 });

	drw.shaderFile.vertexShader = getShader("vert");
	drw.shaderFile.fragmentShader = getShader("frag");
	core.renderer.createShader(&drw.shader, &drw.shaderFile);

	core.renderer.createDepthBuffer(&drw.depthBuffer,false);
	core.renderer.setupDemoRenderPass(&drw.renderPass, drw.depthBuffer);

	core.renderer.createSwapchainImageFramebuffers(&drw.framebuffer, drw.renderPass, drw.depthBuffer);
	
	drw.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
	drw.cprop.blendPreset = { AnthemBlendPreset::AT_ABP_NO_BLEND };
	drw.cprop.vertStageLayout = { drw.vvbuf,drw.vibuf };

	core.renderer.createGraphicsPipelineCustomized(&drw.pipeline, {}, {}, drw.renderPass, drw.shader, drw.vvbuf, &drw.cprop);
}

void recordCommandBufferDrw(int i) {
	auto& renderer = core.renderer;
	renderer.drStartRenderPass(drw.renderPass, (AnthemFramebuffer*)(drw.framebuffer->getFramebufferObject(i)),i, false);
	renderer.drSetViewportScissorFromSwapchain(i);
	renderer.drBindGraphicsPipeline(drw.pipeline,i);
	renderer.drBindVertexBufferMultiple({ drw.vvbuf,drw.vibuf }, i);
	renderer.drBindIndexBuffer(drw.ibuf, i);

	renderer.drDrawInstanced(drw.ibuf->getIndexCount(),MainPipeline::instances, i);
	renderer.drEndRenderPass(i);
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

	std::vector<const AnthemSemaphore*> semaphoreToWait = {};
	std::vector<AtSyncSemaphoreWaitStage> waitStages = {};
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral(currentFrame, imgIdx, &semaphoreToWait, &waitStages);
	core.renderer.drPresentFrame(currentFrame, imgIdx);
	currentFrame++;
	currentFrame %= 2;
}

int main() {
	prepareBaseComponents();
	buildPipeline();
	
	core.renderer.registerPipelineSubComponents();
	recordCommandBufferAll();

	int currentFrame = 0;
	core.renderer.setDrawFunction([&]() {
		drawLoop(currentFrame);
	});
	core.renderer.startDrawLoopDemo();
	core.renderer.finalize();

	return 0;
}