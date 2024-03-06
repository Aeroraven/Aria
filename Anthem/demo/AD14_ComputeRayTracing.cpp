#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"
#include "../include/core/drawing/buffer/impl/AnthemVertexBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemInstancingVertexBufferImpl.h"
#include "../include/core/drawing/image/AnthemImage.h"
#include "../include/core/drawing/buffer/impl/AnthemShaderStorageBufferImpl.h"

using namespace Anthem::External;
using namespace Anthem::Core;
using namespace Anthem::Components::Camera;

class Stage {
public:
	AnthemDescriptorPool* descStorage = nullptr;
	AnthemDescriptorPool** descImage1 = nullptr;
	AnthemDescriptorPool** descStImage1 = nullptr;
	AnthemDescriptorPool* descUni = nullptr;

	AnthemImage** image1 = nullptr;

	AnthemShaderStorageBufferImpl<
		AtBufVecd4f<1>, //Type + Material
		AtBufVecd4f<1>, //Pos
		AtBufVecd4f<1>, //Color
		AtBufVecd4f<1>  //Param
		
	>* ssbo = nullptr;

	AnthemUniformBufferImpl<
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>,
		AnthemUniformVecf<1>
	>* uniform;

	AnthemShaderFilePaths compShaderPath;
	AnthemShaderModule* compShader = nullptr;
	AnthemComputePipeline* compPipe = nullptr;

	AnthemShaderFilePaths dispShaderPath;
	AnthemShaderModule* dispShader = nullptr;
	AnthemSwapchainFramebuffer* framebuffer = nullptr;
	AnthemRenderPass* pass = nullptr;
	AnthemGraphicsPipeline* dispPipe = nullptr;

	AnthemFence** fence = nullptr;
	AnthemSemaphore** semaphore = nullptr;

	AnthemSimpleToyRenderer renderer;
	AnthemConfig config;

	AnthemVertexBufferImpl<
		AtAttributeVecf<2>
	>* vx;
	AnthemIndexBuffer* ix;
	AnthemRenderPassSetupOption opt;
	AnthemDepthBuffer* depth;

	AnthemGraphicsPipelineCreateProps cprop;
	AnthemImagePipelineBarrier tSrc;
	AnthemImagePipelineBarrier tDst;

	uint32_t* compCmdIdx = nullptr;
	const int texSize = 1024;
	const int inFlight = 2;
	const int texSplit = 16;

	float renderedFrames = 0;

}stage;

void initialize() {
	stage.renderer.setConfig(&stage.config);
	stage.renderer.initialize();
}

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "comprt\\comprt.";
	st += x;
	st += ".spv";
	return st;
}

void prepareCompute() {
	stage.descImage1 = new AnthemDescriptorPool * [stage.inFlight];
	stage.descStImage1 = new AnthemDescriptorPool * [stage.inFlight];
	stage.image1 = new AnthemImage * [stage.inFlight];
	for (auto i : std::views::iota(0, stage.inFlight)) {
		stage.renderer.createDescriptorPool(&stage.descImage1[i]);
		stage.renderer.createDescriptorPool(&stage.descStImage1[i]);
		stage.renderer.createTexture(&stage.image1[i], stage.descImage1[i], nullptr, stage.texSize, stage.texSize, 4, 0,
			false, false, AT_IF_SRGB_FLOAT32, -1, false, AT_IU_COMPUTE_OUTPUT);
		stage.image1[i]->toGeneralLayout();

		std::vector<AnthemImageContainer*> ct = { stage.image1[i]};
		stage.renderer.addStorageImageArrayToDescriptor(ct, stage.descStImage1[i], 0, -1);

	}

	stage.renderer.createDescriptorPool(&stage.descUni);
	stage.renderer.createUniformBuffer(&stage.uniform, 0, stage.descUni);

	

	stage.compShaderPath.computeShader = getShader("comp");
	stage.renderer.createShader(&stage.compShader, &stage.compShaderPath);

	stage.renderer.createDescriptorPool(&stage.descStorage);
	using ssboType = std::remove_cv_t<decltype(stage.ssbo)>;
	std::function<void(ssboType)> createFunc = [&](ssboType w) {
		w->setInput(0, { 0,0,0,0 }, { 0,-0.5,4,0 }, { 1,0.8,0.5,1 }, { 1,0,0,0 });
		w->setInput(1, { 1,0,0,0 }, { 0,-1.5,0,0 }, { 1,0.8,0.5,1 }, { 0,1,0,0 });
	};
	stage.renderer.createShaderStorageBuffer(&stage.ssbo, 1, 0, stage.descStorage, std::make_optional(createFunc));
	

	AnthemDescriptorSetEntry dseUni = {
		.descPool = stage.descUni,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseImageF = {
		.descPool = stage.descStImage1[0],
		.descSetType = AT_ACDS_STORAGE_IMAGE,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseImageS = {
		.descPool = stage.descStImage1[1],
		.descSetType = AT_ACDS_STORAGE_IMAGE,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseStore = {
		.descPool = stage.descStorage,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	stage.renderer.createComputePipelineCustomized(&stage.compPipe, { dseUni, dseImageF, dseImageS, dseStore }, stage.compShader);

	stage.fence = new AnthemFence*[stage.inFlight];
	stage.semaphore = new AnthemSemaphore*[stage.inFlight];
	stage.compCmdIdx = new uint32_t[stage.inFlight];
	for (auto i : std::views::iota(0, 2)) {
		stage.renderer.createFence(&stage.fence[i]);
		stage.renderer.createSemaphore(&stage.semaphore[i]);
		stage.renderer.drAllocateCommandBuffer(&stage.compCmdIdx[i]);
	}
}

void updateUniform() {
	int rdH, rdW;
	stage.renderer.exGetWindowSize(rdH, rdW);
	float geomSize = 2.0f, aspectRatio = 1.0f * rdW / rdH, totlSamples = stage.renderedFrames++;
	stage.uniform->specifyUniforms(&geomSize, &aspectRatio, &totlSamples);
	for (auto i : std::views::iota(0, stage.inFlight)) {
		stage.uniform->updateBuffer(i);
	}
}

void prepareDraw() {
	stage.tSrc.access = 0;
	stage.tSrc.layout = VK_IMAGE_LAYOUT_GENERAL;
	stage.renderer.quGetComputeQueueIdx(&stage.tSrc.queueFamily);
	stage.tSrc.stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	stage.tDst.access = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	stage.tDst.layout = VK_IMAGE_LAYOUT_GENERAL;
	stage.renderer.quGetGraphicsQueueIdx(&stage.tDst.queueFamily);
	stage.tDst.stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	stage.renderer.createVertexBuffer(&stage.vx);
	stage.renderer.createIndexBuffer(&stage.ix);

	stage.vx->setTotalVertices(4);
	stage.vx->insertData(0, { -1.0,1.0 });
	stage.vx->insertData(1, { 1.0,1.0 });
	stage.vx->insertData(2, { 1.0,-1.0 });
	stage.vx->insertData(3, { -1.0,-1.0 });

	stage.ix->setIndices({ 0,1,2,2,3,0 });

	stage.dispShaderPath.fragmentShader = getShader("frag");
	stage.dispShaderPath.vertexShader = getShader("vert");
	stage.renderer.createShader(&stage.dispShader, &stage.dispShaderPath);

	stage.renderer.createDepthBuffer(&stage.depth, false);
	stage.opt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	stage.opt.msaaType = AT_ARPMT_NO_MSAA;
	stage.renderer.setupRenderPass(&stage.pass, &stage.opt, stage.depth);

	stage.renderer.createSwapchainImageFramebuffers(&stage.framebuffer, stage.pass, stage.depth);

	AnthemDescriptorSetEntry dseImage = {
		.descPool = stage.descImage1[0],
		.descSetType = AT_ACDS_SAMPLER,
		.inTypeIndex = 0
	};
	stage.renderer.createGraphicsPipelineCustomized(&stage.dispPipe, { dseImage }, stage.pass, stage.dispShader, stage.vx, & stage.cprop);
}

void recordCommandsDisplay() {
	for (auto i : std::views::iota(0, stage.inFlight)) {
		stage.renderer.drStartCommandRecording(i);
		stage.renderer.drColorImagePipelineBarrier(stage.image1[0], &stage.tSrc, &stage.tDst, i);
		stage.renderer.drColorImagePipelineBarrier(stage.image1[1], &stage.tSrc, &stage.tDst, i);
		stage.renderer.drStartRenderPass(stage.pass, (AnthemFramebuffer*)stage.framebuffer->getFramebufferObject(i), i, false);
		stage.renderer.drSetViewportScissor(i);
		stage.renderer.drBindGraphicsPipeline(stage.dispPipe, i);
		stage.renderer.drBindVertexBuffer(stage.vx, i);
		stage.renderer.drBindIndexBuffer(stage.ix, i);
		AnthemDescriptorSetEntry dseImage = {
			.descPool = stage.descImage1[i],
			.descSetType = AT_ACDS_SAMPLER,
			.inTypeIndex = static_cast<uint32_t>(i)
		};
		stage.renderer.drBindDescriptorSetCustomizedGraphics({ dseImage }, stage.dispPipe, i);
		stage.renderer.drDraw(stage.ix->getIndexCount(), i);

		stage.renderer.drEndRenderPass(i);
		stage.renderer.drEndCommandRecording(i);
	}
}

void recordCommandCompute() {
	for (auto i : std::views::iota(0, stage.inFlight)) {
		stage.renderer.drStartCommandRecording(stage.compCmdIdx[i]);
		stage.renderer.drBindComputePipeline(stage.compPipe, stage.compCmdIdx[i]);

		//TODO: Barrier
		AnthemDescriptorSetEntry dseUni = {
			.descPool = stage.descUni,
			.descSetType = AT_ACDS_UNIFORM_BUFFER,
			.inTypeIndex = static_cast<uint32_t>(i)
		};
		AnthemDescriptorSetEntry dseImageF = {
			.descPool = stage.descStImage1[i],
			.descSetType = AT_ACDS_STORAGE_IMAGE,
			.inTypeIndex = static_cast<uint32_t>(i)
		};
		AnthemDescriptorSetEntry dseImageS = {
			.descPool = stage.descStImage1[1-i],
			.descSetType = AT_ACDS_STORAGE_IMAGE,
			.inTypeIndex = static_cast<uint32_t>(i)
		};
		AnthemDescriptorSetEntry dseStore = {
			.descPool = stage.descStorage,
			.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
			.inTypeIndex = static_cast<uint32_t>(i)
		};

		stage.renderer.drBindDescriptorSetCustomizedCompute({ dseUni,dseImageF,dseImageS,dseStore }, stage.compPipe, stage.compCmdIdx[i]);
		stage.renderer.drComputeDispatch(stage.compCmdIdx[i], stage.texSize / stage.texSplit, stage.texSize / stage.texSplit, 1);
		stage.renderer.drEndCommandRecording(stage.compCmdIdx[i]);
	}
}

void mainLoop(int cur) {
	uint32_t iIdx;
	stage.renderer.drPrepareFrame(cur, &iIdx);
	stage.renderer.drSubmitCommandBufferCompQueueGeneral(stage.compCmdIdx[cur], nullptr, nullptr, nullptr);
	stage.renderer.drSubmitBufferPrimaryCall(cur, cur);
	stage.renderer.drPresentFrame(cur, iIdx);
}

int main() {
	initialize();
	prepareCompute();
	prepareDraw();


	stage.renderer.registerPipelineSubComponents();
	recordCommandCompute();
	recordCommandsDisplay();

	uint32_t cFrame = 0;
	stage.renderer.setDrawFunction([&](){
		updateUniform();
		mainLoop(cFrame++);
		cFrame %= 2;
	});
	stage.renderer.startDrawLoopDemo();
	stage.renderer.finalize();

	return 0;
}