#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"
#include "../include/core/drawing/buffer/impl/AnthemVertexBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemInstancingVertexBufferImpl.h"

using namespace Anthem::External;
using namespace Anthem::Core;
using namespace Anthem::Components::Camera;

struct Stage {
	std::vector<AnthemGLTFLoaderParseResult> model;

	AnthemVertexBufferImpl<
		AtAttributeVecf<3>,
		AtAttributeVecf<3>
	>* vx = nullptr;

	AnthemInstancingVertexBufferImpl<
		AtAttributeVecf<1>
	>* vix = nullptr;
	AnthemIndexBuffer* ix = nullptr;
	AnthemDepthBuffer* depth = nullptr;
	AnthemShaderFilePaths shaderPath;
	AnthemShaderModule* shader = nullptr;
	AnthemGraphicsPipeline* pipeline = nullptr;
	AnthemSwapchainFramebuffer* framebuffer = nullptr;
	AnthemRenderPass* renderPass = nullptr;
	AnthemIndirectDrawBuffer* indirect = nullptr;

	AnthemRenderPassSetupOption passOpt;
	AnthemGraphicsPipelineCreateProps cprop;

	using uxProjMat = AnthemUniformMatf<4>;
	using uxModelMat = AnthemUniformMatf<4>;
	using uxViewMat = AnthemUniformMatf<4>;
	AnthemDescriptorPool* descPool;
	AnthemUniformBufferImpl<uxProjMat, uxModelMat, uxViewMat>* ubuf;

	AnthemSimpleToyRenderer renderer;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);

	AnthemDescriptorPool* descImage = nullptr;
	AnthemImage** image = nullptr;

	float rot = 0.0f;
}stage;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "indirect\\shader.";
	st += x;
	st += ".spv";
	return st;
}

void init() {
	stage.renderer.setConfig(&stage.cfg);
	stage.renderer.initialize();
}

void loadModel() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\gsk\\untitled.gltf").c_str());
	loader.parseModel(config, stage.model);
}

void prepare() {
	stage.renderer.createVertexBuffer(&stage.vx);
	stage.renderer.createInstancingBuffer(&stage.vix);

	std::vector<uint32_t> prs;
	uint32_t prx = 0;
	for (auto& p : stage.model) {
		prs.push_back(prx);
		prx += p.numVertices;
	}
	stage.vx->setTotalVertices(prx);
	stage.vix->setTotalVertices(3);
	std::vector<std::thread> th;
	auto childJob = [&](int k)->void {
		for (int i = 0; i < stage.model[k].numVertices; i++) {
			stage.vx->insertData(i + prs[k],
				{ stage.model[k].positions[i * 3],stage.model[k].positions[i * 3 + 1],stage.model[k].positions[i * 3 + 2] },
				{ stage.model[k].texCoords[i * 2],stage.model[k].texCoords[i * 2+1], k*1.0f });
		}

	};
	for (int i = 0; i < 3; i++) {
		stage.vix->insertData(i, { -70.0f+i*70.0f });
	}
	stage.vix->setAttrBindingPoint({ 2 });

	for (int k = 0; k < prs.size(); k++) {
		std::thread childExec(childJob, k);
		th.push_back(std::move(childExec));
	}
	for (int k = 0; k < prs.size(); k++) {
		th[k].join();
	}

	stage.renderer.createIndexBuffer(&stage.ix);
	stage.renderer.createIndirectDrawBuffer(&stage.indirect);

	std::vector<uint32_t> ixList;
	uint32_t alcx = 0;
	for (int k = 0; k < prs.size(); k++) {
		ixList.insert(ixList.end(), stage.model[k].indices.begin(), stage.model[k].indices.end());
		for (int j = 0; j < 3; j++) {
			stage.indirect->addIndirectDrawCommand(3, 0, stage.model[k].indices.size(), alcx, prs[k]);
		}
		alcx += stage.model[k].indices.size();
	}
	stage.ix->setIndices(ixList);

	stage.renderer.createDescriptorPool(&stage.descImage);
	stage.image = new AnthemImage * [stage.model.size()];
	std::vector<AnthemImageContainer*> imgContainer;
	for (int i = 0; i < stage.model.size(); i++) {
		AnthemImageLoader* loader = new AnthemImageLoader();
		uint32_t texWidth, texHeight, texChannels;
		uint8_t* texData;
		std::string texPath = stage.model[i].basePath + stage.model[i].pbrBaseColorTexPath;
		if (stage.model[i].pbrBaseColorTexPath == "") {
			texPath = ANTH_ASSET_DIR;
			texPath += "cat.jpg";
		}
		loader->loadImage(texPath.c_str(), &texWidth, &texHeight, &texChannels, &texData);
		stage.renderer.createTexture(&stage.image[i], stage.descImage, texData, texWidth, texHeight, texChannels, 0, false, false,AT_IF_SRGB_UINT8,-1,true);
		imgContainer.push_back(stage.image[i]);
	}
	
	stage.renderer.addSamplerArrayToDescriptor(imgContainer, stage.descImage, 0, -1);

	stage.shaderPath.vertexShader = getShader("vert");
	stage.shaderPath.fragmentShader = getShader("frag");
	stage.renderer.createShader(&stage.shader, &stage.shaderPath);

	stage.renderer.createDescriptorPool(&stage.descPool);
	stage.renderer.createUniformBuffer(&stage.ubuf, 0, stage.descPool, 0);

	stage.renderer.createDepthBuffer(&stage.depth, false);

	stage.passOpt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	stage.passOpt.msaaType = AT_ARPMT_NO_MSAA;
	stage.renderer.setupRenderPass(&stage.renderPass, &stage.passOpt, stage.depth);

	stage.renderer.createSwapchainImageFramebuffers(&stage.framebuffer, stage.renderPass, stage.depth);

	AnthemDescriptorSetEntry dseUniform = {
		.descPool = stage.descPool,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSampler = {
		.descPool = stage.descImage,
		.descSetType = AT_ACDS_SAMPLER,
		.inTypeIndex = 0
	};
	stage.cprop.vertStageLayout = { stage.vx,stage.vix };
	stage.renderer.createGraphicsPipelineCustomized(&stage.pipeline, { dseUniform,dseSampler }, stage.renderPass, stage.shader, stage.vx, &stage.cprop);
}

void updateUniform() {
	stage.rot += 0.01;
	int rdH, rdW;
	stage.renderer.exGetWindowSize(rdH, rdW);
	stage.camera.specifyFrustum((float)AT_PI / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	stage.camera.specifyPosition(0.0f, 70.0f, -120.0f);
	
	AtMatf4 proj, view, local;
	stage.camera.getProjectionMatrix(proj);
	stage.camera.getViewMatrix(view);
	//local = AnthemLinAlg::eye<float, 4>();
	local = AnthemLinAlg::axisAngleRotationTransform3<float>({ 0.0f,1.0f,0.0f }, stage.rot);

	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);

	stage.ubuf->specifyUniforms(pm, vm, lm);

	for (int i = 0; i < stage.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
		stage.ubuf->updateBuffer(i);
	}
}

void recordCommand() {
	for (int i = 0; i < stage.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
		ANTH_LOGI("Recording Command", i);
		stage.renderer.drStartCommandRecording(i);
		stage.renderer.drStartRenderPass(stage.renderPass,
			(AnthemFramebuffer*)stage.framebuffer->getFramebufferObject(i), i, false);
		stage.renderer.drSetViewportScissor(i);
		stage.renderer.drBindGraphicsPipeline(stage.pipeline, i);
		stage.renderer.drBindVertexBufferMultiple({ stage.vx,stage.vix }, i);
		stage.renderer.drBindIndexBuffer(stage.ix, i);
		AnthemDescriptorSetEntry dseUniform = {
			.descPool = stage.descPool,
			.descSetType = AT_ACDS_UNIFORM_BUFFER,
			.inTypeIndex = 0
		};
		AnthemDescriptorSetEntry dseSampler = {
			.descPool = stage.descImage,
			.descSetType = AT_ACDS_SAMPLER,
			.inTypeIndex = 0
		};
		stage.renderer.drBindDescriptorSetCustomizedGraphics({ dseUniform,dseSampler }, stage.pipeline, i);
		stage.renderer.drDrawIndexedIndirect(stage.indirect, i);
		stage.renderer.drEndRenderPass(i);
		stage.renderer.drEndCommandRecording(i);
	}
}

void drawLoop(int currentFrame) {
	uint32_t imgIdx;
	stage.renderer.drPrepareFrame(currentFrame, &imgIdx);
	stage.renderer.drSubmitBufferPrimaryCall(currentFrame, currentFrame);
	stage.renderer.drPresentFrame(currentFrame, imgIdx);
}

int main() {
	init();
	loadModel();
	prepare();
	ANTH_LOGI("Prepared");
	stage.renderer.registerPipelineSubComponents();
	updateUniform();
	recordCommand();
	int currentFrame = 0;
	stage.renderer.setDrawFunction([&]() {
		updateUniform();
		drawLoop(currentFrame++);
		currentFrame %= 2;
	});
	ANTH_LOGI("Loop Started");
	stage.renderer.startDrawLoopDemo();
	stage.renderer.finalize();
	return 0;
}