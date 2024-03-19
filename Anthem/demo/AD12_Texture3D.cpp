#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"
#include "../include/components/math/AnthemNoise.h"

#include <thread>

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::External;
using namespace Anthem::Components::Camera;
using namespace Anthem::Components::Math;

struct BaseComponents {
	AnthemSimpleToyRenderer renderer;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
}core;

struct Noise {
	unsigned char* buffer = nullptr;
	constexpr static int perDimSize = 100;
	constexpr static int fractalIterations = 5;
	constexpr static int cpuThreads = 1;
}noise;

struct Cube {
	using uProj = AtUniformMatf<4>;
	using uView = AtUniformMatf<4>;
	using uModel = AtUniformMatf<4>;
	using uCamPos = AtUniformVecf<4>;
	AnthemDescriptorPool* descPool;
	AnthemUniformBufferImpl<uProj, uView, uModel, uCamPos>* ubuf;

	AnthemVertexBufferImpl<AnthemVAOAttrDesc<float, 3>>* vbuf;
	AnthemIndexBuffer* ibuf;

	AnthemDepthBuffer* depthBuffer;
	AnthemRenderPass* pass;
	AnthemGraphicsPipeline* pipeline;
	//

	AnthemShaderFilePaths shaderFiles;
	AnthemShaderModule* shader;
	AnthemGraphicsPipelineCreateProps cprop;

	AnthemDescriptorPool* descBack;
	AnthemImage* backfaceLocation;
	AnthemFramebuffer* framebuffer;

	AnthemRenderPassSetupOption opt;
}cube;

struct VolumetricNoise {
	AnthemDescriptorPool* descNoise;
	AnthemImage* volNoise;
	AnthemSwapchainFramebuffer* framebuffer;

	AnthemShaderFilePaths shaderFiles;
	AnthemShaderModule* shader;
	AnthemGraphicsPipelineCreateProps cprop;
	AnthemDepthBuffer* depthBuffer;
	AnthemRenderPass* pass;
	AnthemGraphicsPipeline* pipeline;

	AnthemRenderPassSetupOption opt;

}vol;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "tex3d\\shader.";
	st += x;
	st += ".spv";
	return st;
}

void prepareRenderer() {
	core.renderer.setConfig(&core.cfg);
	core.renderer.initialize();
	core.camera.specifyPosition(0.0, 0.0, -4.0);
}

void loadNoise() {
	noise.buffer = new unsigned char[noise.perDimSize * noise.perDimSize * noise.perDimSize];
	auto jobFunction = [&](int jobId, int totalJobs) {
		constexpr static float stepInterval = 0.007;
		IAnthemNoiseGenerator* baseGenerator = new AnthemImprovedNoise();
		AnthemFractalNoise* noiseGenerator = new AnthemFractalNoise(baseGenerator,noise.fractalIterations);
		int segmentSize = noise.perDimSize * noise.perDimSize * noise.perDimSize / totalJobs;
		for (auto i : std::views::iota(jobId* segmentSize, (jobId + 1)* segmentSize)) {
			int px = i / (noise.perDimSize * noise.perDimSize);
			int py = (i % (noise.perDimSize * noise.perDimSize)) / noise.perDimSize;
			int pz = (i % (noise.perDimSize * noise.perDimSize)) %  noise.perDimSize;
			double cp = noiseGenerator->noise(px * stepInterval, py * stepInterval, pz * stepInterval) * 255.0;
			noise.buffer[i] = (uint8_t)(cp);
		}
	};
	std::vector<std::thread> runningJobs;
	for (auto i : std::views::iota(0, noise.cpuThreads)) {
		auto w = std::thread(jobFunction, i, noise.cpuThreads);
		runningJobs.push_back(std::move(w));
	}
	for (auto& p : runningJobs) {
		p.join();
	}
}

void prepareSecondPass() {
	core.renderer.createDescriptorPool(&vol.descNoise);
	core.renderer.createTexture3d(&vol.volNoise, vol.descNoise, noise.buffer, noise.perDimSize, noise.perDimSize, noise.perDimSize, 1, 0,AT_IF_R_UINT8);

	vol.shaderFiles.vertexShader = getShader("second.vert");
	vol.shaderFiles.fragmentShader = getShader("second.frag");
	core.renderer.createShader(&vol.shader, &vol.shaderFiles);

	core.renderer.createDepthBuffer(&vol.depthBuffer,false);
	vol.opt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	vol.opt.msaaType = AT_ARPMT_NO_MSAA;
	core.renderer.setupRenderPass(&vol.pass, &vol.opt, vol.depthBuffer);

	core.renderer.createSwapchainImageFramebuffers(&vol.framebuffer, vol.pass, vol.depthBuffer);

	AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
		.descPool = cube.descPool,
		.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry samplerRdw = {
		.descPool = cube.descBack,
		.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry samplerNoiseRdw = {
		.descPool = vol.descNoise,
		.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descEntry = { uniformBufferDescEntryRdw,samplerRdw,samplerNoiseRdw };
	core.renderer.createGraphicsPipelineCustomized(&vol.pipeline, descEntry, {}, vol.pass, vol.shader, cube.vbuf, &vol.cprop);

}

void createCube() {
	static unsigned pv[6] = { 0,1,2,0,2,3 };
	core.renderer.createVertexBuffer(&cube.vbuf);
	core.renderer.createIndexBuffer(&cube.ibuf);

	cube.vbuf->setTotalVertices(24);
	std::vector<unsigned> idx;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 4; k++) {
				std::array<float, 3> dst;
				int fixed = i;
				int dyna1 = (i !=0) ? 0 : 1;
				int dyna2 = 3 - fixed - dyna1;
				dst[fixed] = j ? 1 : -1;
				dst[dyna1] = (k == 0 || k == 3) ? -1 : 1;
				dst[dyna2] = (k == 0 || k == 1) ? 1 : -1;
				cube.vbuf->insertData(i * 8 + j * 4 + k, dst);
			}
			unsigned baseOffset = i * 8 + j * 4;
			for (int k = 0; k < 6; k++) {
				idx.push_back(baseOffset + pv[k]);
			}
		}
	}
	cube.ibuf->setIndices(idx);
	core.renderer.createDescriptorPool(&cube.descBack);
	core.renderer.createColorAttachmentImage(&cube.backfaceLocation, cube.descBack, 0, AT_IF_SBGR_UINT8, false);

	core.renderer.createDepthBuffer(&cube.depthBuffer, false);
	core.renderer.createDescriptorPool(&cube.descPool);
	core.renderer.createUniformBuffer(&cube.ubuf,0,cube.descPool);

	cube.shaderFiles.vertexShader = getShader("first.vert");
	cube.shaderFiles.fragmentShader = getShader("first.frag");
	core.renderer.createShader(&cube.shader, &cube.shaderFiles);
	
	//core.renderer.setupDemoRenderPass(&cube.pass, cube.depthBuffer);
	//core.renderer.createSwapchainImageFramebuffers(&cube.framebuffer, cube.pass, cube.depthBuffer);

	cube.opt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	cube.opt.msaaType = AT_ARPMT_NO_MSAA;
	core.renderer.setupRenderPass(&cube.pass, &cube.opt, cube.depthBuffer);

	std::vector<const AnthemImage*> colorAttachments = {cube.backfaceLocation};
	core.renderer.createSimpleFramebuffer(&cube.framebuffer, &colorAttachments, cube.pass, cube.depthBuffer);
	
	AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
			.descPool = cube.descPool,
			.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
			.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descEntry = { uniformBufferDescEntryRdw };
	core.renderer.createGraphicsPipelineCustomized(&cube.pipeline, descEntry, {}, cube.pass, cube.shader, cube.vbuf, &cube.cprop);

}

void recordCube() {
	for (int i = 0; i < core.cfg.vkcfgMaxImagesInFlight; i++) {
		core.renderer.drStartCommandRecording(i);
		
		// Pass 1: Endpoint of ray
		core.renderer.drStartRenderPass(cube.pass, cube.framebuffer, i, false);
		core.renderer.drSetViewportScissorFromSwapchain(i);
		core.renderer.drBindGraphicsPipeline(cube.pipeline, i);
		core.renderer.drBindVertexBuffer(cube.vbuf, i);
		core.renderer.drBindIndexBuffer(cube.ibuf, i);

		AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
				.descPool = cube.descPool,
				.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
				.inTypeIndex = 0
		};
		std::vector<AnthemDescriptorSetEntry> descEntry = { uniformBufferDescEntryRdw };

		core.renderer.drBindDescriptorSetCustomizedGraphics(descEntry, cube.pipeline, i);
		core.renderer.drDraw(cube.ibuf->getIndexCount(), i);
		core.renderer.drEndRenderPass(i);

		// Pass 2: 
		core.renderer.drStartRenderPass(vol.pass, (AnthemFramebuffer*)vol.framebuffer->getFramebufferObject(i), i, false);
		core.renderer.drSetViewportScissorFromSwapchain(i);
		core.renderer.drBindGraphicsPipeline(vol.pipeline, i);
		core.renderer.drBindVertexBuffer(cube.vbuf, i);
		core.renderer.drBindIndexBuffer(cube.ibuf, i);

		AnthemDescriptorSetEntry uniformBufferDescEntryRdw2 = {
			.descPool = cube.descPool,
			.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
			.inTypeIndex = 0
		};
		AnthemDescriptorSetEntry samplerRdw = {
			.descPool = cube.descBack,
			.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
			.inTypeIndex = 0
		};
		AnthemDescriptorSetEntry samplerNoiseRdw = {
			.descPool = vol.descNoise,
			.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
			.inTypeIndex = 0
		};
		std::vector<AnthemDescriptorSetEntry> descEntry2 = { uniformBufferDescEntryRdw2,samplerRdw,samplerNoiseRdw };
		core.renderer.drBindDescriptorSetCustomizedGraphics(descEntry2, vol.pipeline, i);
		core.renderer.drDraw(cube.ibuf->getIndexCount(), i);
		core.renderer.drEndRenderPass(i);

		core.renderer.drEndCommandRecording(i);
	}
}

void updateUniform(int frameId) {
	AtMatf4 proj, view, local;
	int rdH, rdW;
	core.renderer.exGetWindowSize(rdH, rdW);
	core.camera.specifyFrustum(AT_PI / 3 * 1, 0.1, 100, 1.0 * rdW / rdH);

	core.camera.getProjectionMatrix(proj);
	core.camera.getViewMatrix(view);
	local = AnthemLinAlg::eye<float, 4>();
	local = local.multiply(Math::AnthemLinAlg::axisAngleRotationTransform3(Math::AnthemVector<float, 3>({ .0f,01.0f,0.0f }),glfwGetTime()*0.1f));

	float rProj[16], rView[16], rLocal[16], rCamPos[4];
	proj.columnMajorVectorization(rProj);
	view.columnMajorVectorization(rView);
	local.columnMajorVectorization(rLocal);
	rCamPos[0] = 0;
	rCamPos[1] = 0;
	rCamPos[2] = -4.0;
	
	cube.ubuf->specifyUniforms(rProj, rView, rLocal,rCamPos);
	cube.ubuf->updateBuffer(frameId);
}

void drawCall(int frameId) {
	uint32_t imgIdx;
	core.renderer.drPrepareFrame(frameId, &imgIdx);

	std::vector<const AnthemSemaphore*> signalWait = {  };
	std::vector<AtSyncSemaphoreWaitStage> waitStages = {};
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral(frameId, imgIdx, &signalWait, &waitStages);
	core.renderer.drPresentFrame(frameId, imgIdx);
}

int main() {
	prepareRenderer();
	loadNoise();
	createCube();
	prepareSecondPass();

	core.renderer.registerPipelineSubComponents();
	recordCube();

	int curIdx = 0;
	core.renderer.setDrawFunction([&]() {
		updateUniform(curIdx);
		drawCall(curIdx);
		curIdx = (curIdx + 1) % 2;
	});
	core.renderer.startDrawLoopDemo();
	core.renderer.finalize();
	return 0;
}