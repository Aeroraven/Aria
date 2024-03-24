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
#include "../include/components/utility/AnthemSimpleModelIntegrator.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::External;
using namespace Anthem::Core;


struct Stage {
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemFrameRateMeter fpsMeter = AnthemFrameRateMeter(10);

	std::vector<AnthemGLTFLoaderParseResult> model;
	AnthemSimpleModelIntegrator ldModel;
	AnthemSimpleModelIntegratorRayTracingStructs modelRt;

	// Storage Image
	AnthemImage* storageImage = nullptr;
	AnthemDescriptorPool* descImg = nullptr;
	uint32_t imgHeight = 0;
	uint32_t imgWidth = 0;

	// Uniform Buffer
	AnthemDescriptorPool* descUniform = nullptr;
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>>* uniformBuffer = nullptr;

	// Pipeline
	AnthemDescriptorPool* descAs = nullptr;
	AnthemRayTracingShaders* shader = nullptr;
	AnthemRayTracingPipeline* pipeline = nullptr;

	// Image
	AnthemDescriptorPool* descImage = nullptr;
	AnthemImage** image = nullptr;

}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "rtmodel\\rtmodel.";
	st += x;
	st += ".hlsl.spv";
	return st;
}


void initialize() {
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.cfg.demoName = "24. Ray-tracing-based Model Drawing";
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();

	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);

	st.camera.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 90, 100);
	st.camera.specifyFrontEyeRay(0, -0.0, -1);
}


void loadModel() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\klee\\untitled.gltf").c_str());
	loader.parseModel(config, st.model);
	st.model.pop_back();
	std::vector<AnthemUtlSimpleModelStruct> rp;
	for (auto& p : st.model)rp.push_back(p);

	// Add Ground
	AnthemUtlSimpleModelStruct ground;
	float dx = 80.0, dy = 5.0;
	ground.positions = { -dx,dy,-dx,-dx,dy,dx,dx,dy,dx,dx,dy,-dx };
	ground.normals = { 0,1,0,0,1,0,0,1,0,0,1,0 };
	ground.texCoords = { 0,0,0,0,0,0,0,0 };
	ground.indices = { 0,1,2,2,3,0 };
	rp.push_back(ground);

	st.ldModel.loadModelRayTracing(&st.rd, rp, { 3 });
	st.modelRt = st.ldModel.getRayTracingParsedResult();

	st.rd.createDescriptorPool(&st.descImage);
	st.image = new AnthemImage * [st.model.size()];
	std::vector<AnthemImageContainer*> imgContainer;
	for (int i = 0; i < st.model.size(); i++) {
		AnthemImageLoader* loader = new AnthemImageLoader();
		uint32_t texWidth, texHeight, texChannels;
		uint8_t* texData;
		std::string texPath = st.model[i].basePath + st.model[i].pbrBaseColorTexPath;
		if (st.model[i].pbrBaseColorTexPath == "") {
			texPath = ANTH_ASSET_DIR;
			texPath += "cat.jpg";
		}
		loader->loadImage(texPath.c_str(), &texWidth, &texHeight, &texChannels, &texData);
		st.rd.createTexture(&st.image[i], st.descImage, texData, texWidth, texHeight, texChannels, 0, false, false, AT_IF_SRGB_UINT8, -1, true);
		imgContainer.push_back(st.image[i]);
	}
	st.rd.addSamplerArrayToDescriptor(imgContainer, st.descImage, 0, -1);
}


void createUniformBuffer() {
	st.rd.createDescriptorPool(&st.descUniform);
	st.rd.createUniformBuffer(&st.uniformBuffer, 0, st.descUniform, -1);
	AtMatf4 proj, view, local;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);

	proj = AnthemLinAlg::gaussJordan(proj);
	view = AnthemLinAlg::gaussJordan(view);

	float pm[16], vm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);

	st.uniformBuffer->specifyUniforms(pm, vm);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniformBuffer->updateBuffer(i);
	}
}

void initTLAS() {
	st.rd.createDescriptorPool(&st.descAs);
	st.rd.addTopLevelASToDescriptor(st.modelRt.tlasObj, st.descAs, 0, -1);
}

void createPipeline() {
	st.rd.createRayTracingShaderGroup(&st.shader, {
		{AT_RTSG_RAYGEN,{{getShader("rgen"),AT_RTST_RAYGEN}}},
		{AT_RTSG_MISS,{{getShader("rmiss"),AT_RTST_MISS}}},
		{AT_RTSG_MISS,{{getShader("shadow.rmiss"),AT_RTST_MISS}}},
		{AT_RTSG_HIT,{{getShader("rchit"),AT_RTST_CLOSEHIT}}},
	});
	
	AnthemDescriptorSetEntry dseAs{ st.descAs,AT_ACDS_ACC_STRUCT,0 };
	AnthemDescriptorSetEntry dseImg{ st.descImg,AT_ACDS_STORAGE_IMAGE,0 };
	AnthemDescriptorSetEntry dseUni{ st.descUniform,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseTex{ st.descImage,AT_ACDS_SAMPLER ,0 };
	AnthemDescriptorSetEntry dsePos{ st.modelRt.descPos,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dseNormal{ st.modelRt.descNormal,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dseTexCoord{ st.modelRt.descTex,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dseIdx{ st.modelRt.descIndex,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dseOff{ st.modelRt.descOffset,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	st.rd.createRayTracingPipeline(&st.pipeline, { dseAs,dseImg,dseUni,dseTex,dsePos,dseNormal,dseTexCoord,dseIdx,dseOff }, {}, st.shader, 1);
}

void recordCommands() {
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.rd.drStartCommandRecording(i);
		// Ray Tracing
		st.rd.drBindRayTracingPipeline(st.pipeline, i);
		AnthemDescriptorSetEntry dseAs{ st.descAs,AT_ACDS_ACC_STRUCT,0 };
		AnthemDescriptorSetEntry dseImg{ st.descImg,AT_ACDS_STORAGE_IMAGE,0 };
		AnthemDescriptorSetEntry dseUni{ st.descUniform,AT_ACDS_UNIFORM_BUFFER,0 };
		AnthemDescriptorSetEntry dseTex{ st.descImage,AT_ACDS_SAMPLER ,0 };
		AnthemDescriptorSetEntry dsePos{ st.modelRt.descPos,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dseNormal{ st.modelRt.descNormal,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dseTexCoord{ st.modelRt.descTex,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dseIdx{ st.modelRt.descIndex,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dseOff{ st.modelRt.descOffset,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		st.rd.drBindDescriptorSetCustomizedRayTracing({ dseAs,dseImg,dseUni,dseTex,dsePos,dseNormal,dseTexCoord,dseIdx,dseOff }, st.pipeline, i);

		uint32_t sH, sW;
		st.rd.getSwapchainImageExtent(&sW, &sH);
		st.rd.drTraceRays(st.pipeline, sH, sW, i);

		// Present
		st.rd.drSetImageLayoutSimple(st.storageImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i);
		st.rd.drSetSwapchainImageLayoutSimple(i, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i);
		st.rd.drCopyImageToSwapchainImage(st.storageImage, i, i);
		st.rd.drSetImageLayoutSimple(st.storageImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, i);
		st.rd.drSetSwapchainImageLayoutSimple(i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, i);
		st.rd.drEndCommandRecording(i);
	}
}

void createStorageImage() {
	st.rd.getSwapchainImageExtent(&st.imgWidth, &st.imgHeight);
	st.rd.createTexture(&st.storageImage, nullptr, nullptr,
		st.imgWidth, st.imgHeight, 4, 0, false, false, AT_IF_SWAPCHAIN, -1, true, AT_IU_RAYTRACING_DEST);
	st.storageImage->toGeneralLayout();

	st.rd.createDescriptorPool(&st.descImg);
	st.rd.addStorageImageArrayToDescriptor({ st.storageImage }, st.descImg, 0, -1);
}

void mainLoop() {
	st.fpsMeter.record();
	static int fx = 0;
	fx = (fx + 1) % 100;
	if (fx == 0) {
		ANTH_LOGI("FPS:", st.fpsMeter.getFrameRate());
	}

	static int cur = 0;
	uint32_t imgIdx;
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.rd.drSubmitBufferPrimaryCall(cur, cur);
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}

int main() {
	initialize();
	loadModel();
	createStorageImage();
	createUniformBuffer();
	initTLAS();
	createPipeline();
	
	st.rd.registerPipelineSubComponents();
	recordCommands();

	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();

	st.rd.finalize();
	return 0;
}