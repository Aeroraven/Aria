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

#include "../include/core/drawing/buffer/impl/AnthemPushConstantImpl.h"

using namespace Anthem::External;
using namespace Anthem::Core;
using namespace Anthem::Components::Camera;

struct Stage {
	//General
	AnthemSimpleToyRenderer renderer;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemConfig config;

	//Cloth
	AnthemDescriptorPool* descUni;
	AnthemDescriptorPool* descSsbo;
	AnthemShaderStorageBufferImpl<
		AtBufVecd4f<1>, //Pos
		AtBufVecd4f<1>, //Vel
		AtBufVecd4f<1> //LastPos
	>* ssbo;
	AnthemUniformBufferImpl<
		AtUniformVecf<1>, //Cloth Size
		AtUniformVecf<1>, //Nei Dist
		AtUniformVecf<1>, //k
		AtUniformVecf<1>, //g
		AtUniformVecf<1>, //m
		AtUniformVecf<1>, //Damping
		AtUniformVecf<1> //TimeStep
	>* uniform;
	AnthemIndexBuffer* ix;

	//Compute
	AnthemShaderFilePaths compShaderPath;
	AnthemShaderModule* compShader;
	AnthemComputePipeline* compPipe;
	AnthemSemaphore** compComplete;
	AnthemFence* compFence;
	uint32_t* compCmd;

	//Graphics
	AnthemPushConstantImpl<
		AtBufMat4f<1>,
		AtBufMat4f<1>,
		AtBufMat4f<1>
	>* pconst;
	AnthemShaderFilePaths shaderPath;
	AnthemShaderModule* shader;
	AnthemGraphicsPipeline* dispPipe;
	AnthemRenderPass* pass;
	AnthemSwapchainFramebuffer* framebuffer;
	AnthemDepthBuffer* depth;

	AnthemRenderPassSetupOption opt;
	AnthemGraphicsPipelineCreateProps cprop;

	//Consts
	const int clothWidth = 64;
	const int loclSize = 2;
	const int inFlight = 2;
	const int clIters = 64;
}stage;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "compcl\\compcl.";
	st += x;
	st += ".spv";
	return st;
}

void initialize() {
	stage.renderer.setConfig(&stage.config);
	stage.renderer.initialize();
	int rdH, rdW;
	stage.renderer.exGetWindowSize(rdH, rdW);
	stage.camera.specifyFrustum((float)AT_PI / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	stage.camera.specifyPosition(0, 0, -2);
}

void updatePushConstant() {
	stage.camera.specifyPosition(0.0f, 0.0f, -2.0f);

	AtMatf4 proj, view, local;
	stage.camera.getProjectionMatrix(proj);
	stage.camera.getViewMatrix(view);
	local = AnthemLinAlg::axisAngleRotationTransform3<float>({ 0.0f,1.0f,0.0f }, 0.0);

	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);

	stage.pconst->setConstant(pm, vm, lm);
	stage.pconst->enableShaderStage(AT_APCS_VERTEX);
	stage.pconst->enableShaderStage(AT_APCS_FRAGMENT);
}

void prepareCloth() {
	stage.renderer.createDescriptorPool(&stage.descSsbo);
	using ssboType = std::remove_cv_t<decltype(stage.ssbo)>;
	std::function<void(ssboType)> createFunc = [&](ssboType w) {
		
		for (int j = 0; j < stage.clothWidth; j++) {
			for (int i = 0; i < stage.clothWidth; i++) {
				int curId = j * stage.clothWidth + i;
				float cx = (i * 1.0f / (stage.clothWidth - 1)) * 1.0f - 0.5f;
				float cy = (j * 1.0f / (stage.clothWidth - 1)) * 1.0f - 0.5f;
				w->setInput(curId, { cx,cy,0.0,1.0f }, { cx,cy,0.0f,1.0f }, { cx,cy,0.0,1.0f });
			}
		}
	};
	stage.renderer.createShaderStorageBuffer(&stage.ssbo, stage.clothWidth * stage.clothWidth, 0, stage.descSsbo, std::make_optional(createFunc), -1);
	std::vector<uint32_t> idc;
	for (int i = 0; i < stage.clothWidth - 1; i++) {
		for (int j = 0; j < stage.clothWidth - 1; j++) {
			uint32_t lc = i * stage.clothWidth + j;
			uint32_t rc = i * stage.clothWidth + j + 1;
			uint32_t lcx = (i+1)*stage.clothWidth + j;
			uint32_t rcx = (i + 1) * stage.clothWidth + j + 1;
			idc.push_back(lc);
			idc.push_back(rc);
			idc.push_back(rcx);
			idc.push_back(rcx);
			idc.push_back(lcx);
			idc.push_back(lc);
		}
	}

	stage.renderer.createIndexBuffer(&stage.ix);
	stage.ix->setIndices(idc);

	stage.renderer.createDescriptorPool(&stage.descUni);
	stage.renderer.createUniformBuffer(&stage.uniform, 0, stage.descUni);

	float ndist = 1.0f / (stage.clothWidth - 1);
	float sK = 1000.0f;
	float sG = 0.1f;
	float sM = 1.0f;
	float sDamping = 0.95;
	float sTimestep = 0.005;
	float clothWid = stage.clothWidth;
	stage.uniform->specifyUniforms(&clothWid, &ndist, &sK, &sG, &sM, &sDamping, &sTimestep);
	for (int i = 0; i < stage.inFlight; i++) {
		stage.uniform->updateBuffer(i);
	}
}

void prepareCompute() {
	stage.compShaderPath.computeShader = getShader("comp");
	stage.renderer.createShader(&stage.compShader, &stage.compShaderPath);

	AnthemDescriptorSetEntry dseIn = {
		.descPool = stage.descSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseOut = {
		.descPool = stage.descSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseUni = {
		.descPool = stage.descUni,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	stage.renderer.createComputePipelineCustomized(&stage.compPipe, { dseIn,dseOut,dseUni }, stage.compShader);

	stage.compComplete = new AnthemSemaphore*[stage.inFlight];
	stage.compCmd = new uint32_t[stage.inFlight];
	stage.renderer.createFence(&stage.compFence);
	for (int i = 0; i < stage.inFlight; i++) {
		stage.renderer.drAllocateCommandBuffer(&stage.compCmd[i]);
		stage.renderer.createSemaphore(&stage.compComplete[i]);
	}

}

void prepareGraphics() {
	stage.renderer.createPushConstant(&stage.pconst);
	
	stage.shaderPath.vertexShader = getShader("vert");
	stage.shaderPath.fragmentShader = getShader("frag");
	stage.renderer.createShader(&stage.shader, &stage.shaderPath);

	stage.opt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	stage.opt.msaaType = AT_ARPMT_NO_MSAA;

	stage.renderer.createDepthBuffer(&stage.depth,false);
	stage.renderer.setupRenderPass(&stage.pass, &stage.opt, stage.depth);

	stage.renderer.createSwapchainImageFramebuffers(&stage.framebuffer, stage.pass, stage.depth);

	stage.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
	updatePushConstant();
	stage.renderer.createGraphicsPipelineCustomized(&stage.dispPipe, {}, {stage.pconst}, stage.pass, stage.shader, stage.ssbo, &stage.cprop);
}

void recordCommandDisplay() {
	for (auto i : std::views::iota(0, stage.inFlight)) {
		stage.renderer.drStartCommandRecording(i);
		stage.renderer.drStartRenderPass(stage.pass, (AnthemFramebuffer*)(stage.framebuffer->getFramebufferObject(i)), i, false);
		stage.renderer.drBindGraphicsPipeline(stage.dispPipe, i);
		stage.renderer.drBindVertexBufferFromSsbo(stage.ssbo, i, i);
		stage.renderer.drBindIndexBuffer(stage.ix,i);
		stage.renderer.drPushConstants(stage.pconst, stage.dispPipe, i);
		stage.renderer.drSetViewportScissor(i);
		stage.renderer.drDraw(stage.ix->getIndexCount(), i);
		stage.renderer.drEndRenderPass(i);
		stage.renderer.drEndCommandRecording(i);
	}
}

void recordCommandCompute() {

	AnthemBufferBarrierProp c2cSrc = {
		.access = VK_ACCESS_SHADER_WRITE_BIT,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.queueFamily = VK_QUEUE_FAMILY_IGNORED,
	};
	AnthemBufferBarrierProp c2cDst = {
		.access = VK_ACCESS_SHADER_READ_BIT,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.queueFamily = VK_QUEUE_FAMILY_IGNORED
	};
	AnthemBufferBarrierProp c2gSrc = {
		.access = VK_ACCESS_SHADER_WRITE_BIT,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.queueFamily = VK_QUEUE_FAMILY_IGNORED
	};
	AnthemBufferBarrierProp c2gDst = {
		.access = VK_ACCESS_SHADER_READ_BIT,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.queueFamily = VK_QUEUE_FAMILY_IGNORED
	};
	stage.renderer.quGetComputeQueueIdx(&c2gSrc.queueFamily);
	stage.renderer.quGetGraphicsQueueIdx(&c2gDst.queueFamily);


	for (auto i : std::views::iota(0, stage.inFlight)) {
		auto cmdIdx = stage.compCmd[i];
		stage.renderer.drStartCommandRecording(cmdIdx);
		stage.renderer.drBindComputePipeline(stage.compPipe, cmdIdx);
		for (int k = 0; k < stage.clIters; k++) {
			AnthemDescriptorSetEntry dseIn = {
				.descPool = stage.descSsbo,
				.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
				.inTypeIndex = static_cast<uint32_t>(k%2)
			};
			AnthemDescriptorSetEntry dseOut = {
				.descPool = stage.descSsbo,
				.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
				.inTypeIndex = static_cast<uint32_t>(1 - k%2)
			};
			AnthemDescriptorSetEntry dseUni = {
				.descPool = stage.descUni,
				.descSetType = AT_ACDS_UNIFORM_BUFFER,
				.inTypeIndex = 0
			};
			stage.renderer.drBindDescriptorSetCustomizedCompute({ dseIn,dseOut,dseUni }, stage.compPipe, cmdIdx);
			stage.renderer.drComputeDispatch(cmdIdx, stage.clothWidth / stage.loclSize, stage.clothWidth / stage.loclSize, 1);
			if (k != stage.clIters - 1) {
				stage.renderer.drStorageBufferPipelineBarrier(stage.ssbo, 0, &c2cSrc, &c2cDst, cmdIdx);
				stage.renderer.drStorageBufferPipelineBarrier(stage.ssbo, 1, &c2cSrc, &c2cDst, cmdIdx);
			}
		}
		//stage.renderer.drStorageBufferPipelineBarrier(stage.ssbo, 0, &c2gSrc, &c2gDst, cmdIdx);
		//stage.renderer.drStorageBufferPipelineBarrier(stage.ssbo, 1, &c2gSrc, &c2gDst, cmdIdx);
		stage.renderer.drEndCommandRecording(cmdIdx);
	}
}

void mainLoop(int cur) {
	uint32_t imageIdx;
	stage.renderer.drPrepareFrame(cur, &imageIdx);

	stage.compFence->waitForFence();
	stage.compFence->resetFence();

	std::vector<const AnthemSemaphore*> compSemToSignal = { stage.compComplete[0] };
	stage.renderer.drSubmitCommandBufferCompQueueGeneral(stage.compCmd[0], nullptr, &compSemToSignal, stage.compFence);

	std::vector<const AnthemSemaphore*> drawSemToWait = { stage.compComplete[0] };
	std::vector<AtSyncSemaphoreWaitStage> drawSemWaitStages = { AtSyncSemaphoreWaitStage::AT_SSW_VERTEX_INPUT };
	stage.renderer.drSubmitCommandBufferGraphicsQueueGeneral(cur, cur, &drawSemToWait, &drawSemWaitStages);

	stage.renderer.drPresentFrame(cur, imageIdx);
}

int main() {
	initialize();
	prepareCloth();
	prepareCompute();
	prepareGraphics();
	stage.renderer.registerPipelineSubComponents();

	recordCommandCompute();
	recordCommandDisplay();

	int cur = 0;
	stage.renderer.setDrawFunction([&]() {
		mainLoop(cur++);
		cur %= 2;
	});
	stage.renderer.startDrawLoopDemo();
	stage.renderer.finalize();

	return 0;
}