#define AT_FEATURE_RAYTRACING_ENABLED
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

	AnthemDescriptorPool* descCounter = nullptr;
	AnthemUniformBufferImpl<AtUniformVecf<1>, AtUniformVecf<1>, AtUniformVecf<1>, AtUniformVecf<1>>* uniCounter = nullptr;
	float counter = 0;

	// Pipeline
	AnthemDescriptorPool* descAs = nullptr;
	AnthemRayTracingShaders* shader = nullptr;
	AnthemRayTracingPipeline* pipeline = nullptr;


}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "cornell\\mcpt.";
	st += x;
	st += ".hlsl.spv";
	return st;
}


void initialize() {
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.cfg.demoName = "25-B. Monte Carlo Path Tracer / Cornell Box";
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();

	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);

	st.camera.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, -2, 1);
	st.camera.specifyUp(0, 0, 1);
	st.camera.specifyFrontEyeRay(0, 1, 0.001);
}

void loadModel() {
	AnthemGLTFLoader loader;
	AnthemGLTFLoaderParseConfig config;
	std::string path = ANTH_ASSET_DIR;
	loader.loadModel((path + "\\cornell\\untitled.gltf").c_str());
	loader.parseModel(config, st.model);
	std::vector<AnthemUtlSimpleModelStruct> rp;
	for (auto& p : st.model)rp.push_back(p);

	st.ldModel.loadModelRayTracing(&st.rd, rp, { 4 });
	st.modelRt = st.ldModel.getRayTracingParsedResult();
}


void createUniformBuffer() {
	st.rd.createDescriptorPool(&st.descUniform);
	st.rd.createUniformBuffer(&st.uniformBuffer, 0, st.descUniform, -1);
	st.rd.createDescriptorPool(&st.descCounter);
	st.rd.createUniformBuffer(&st.uniCounter, 0, st.descCounter, -1);

	AtMatf4 proj, view, local;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);

	proj = AnthemLinAlg::gaussJordan(proj);
	view = AnthemLinAlg::gaussJordan(view);

	float pm[16], vm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);

	st.uniformBuffer->specifyUniforms(pm, vm);
	float totalLights = st.ldModel.getLightFaces(), totalAreas = st.ldModel.getLightAreas();
	float timex = 0;
	st.uniCounter->specifyUniforms(&st.counter, &totalLights, &totalAreas, &timex);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniformBuffer->updateBuffer(i);
		st.uniCounter->updateBuffer(i);
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
		{AT_RTSG_HIT,{{getShader("rchit"),AT_RTST_CLOSEHIT}}},
		});

	AnthemDescriptorSetEntry dseAs{ st.descAs,AT_ACDS_ACC_STRUCT,0 };
	AnthemDescriptorSetEntry dseImg{ st.descImg,AT_ACDS_STORAGE_IMAGE,0 };
	AnthemDescriptorSetEntry dseUni{ st.descUniform,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseColor{ st.modelRt.descColor,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dsePos{ st.modelRt.descPos,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dseNormal{ st.modelRt.descNormal,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dseIdx{ st.modelRt.descIndex,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dseOff{ st.modelRt.descOffset,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	AnthemDescriptorSetEntry dseUniCounter{ st.descCounter,AT_ACDS_UNIFORM_BUFFER,0 };
	AnthemDescriptorSetEntry dseLightTri{ st.modelRt.descLightIdx,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
	st.rd.createRayTracingPipeline(&st.pipeline,
		{ dseAs,dseImg,dseUni,dseColor,dsePos,dseNormal,dseIdx,dseOff,dseUniCounter,dseLightTri },
		{}, st.shader, 9);
}

void recordCommands() {
	for (auto i : AT_RANGE2(st.cfg.vkcfgMaxImagesInFlight)) {
		st.rd.drStartCommandRecording(i);
		// Ray Tracing
		st.rd.drBindRayTracingPipeline(st.pipeline, i);
		AnthemDescriptorSetEntry dseAs{ st.descAs,AT_ACDS_ACC_STRUCT,0 };
		AnthemDescriptorSetEntry dseImg{ st.descImg,AT_ACDS_STORAGE_IMAGE,0 };
		AnthemDescriptorSetEntry dseUni{ st.descUniform,AT_ACDS_UNIFORM_BUFFER,0 };
		AnthemDescriptorSetEntry dseColor{ st.modelRt.descColor,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dsePos{ st.modelRt.descPos,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dseNormal{ st.modelRt.descNormal,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dseIdx{ st.modelRt.descIndex,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dseOff{ st.modelRt.descOffset,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		AnthemDescriptorSetEntry dseUniCounter{ st.descCounter,AT_ACDS_UNIFORM_BUFFER,0 };
		AnthemDescriptorSetEntry dseLightTri{ st.modelRt.descLightIdx,AT_ACDS_SHADER_STORAGE_BUFFER ,0 };
		st.rd.drBindDescriptorSetCustomizedRayTracing(
			{ dseAs,dseImg,dseUni,dseColor,dsePos,dseNormal,dseIdx,dseOff,dseUniCounter,dseLightTri }, st.pipeline, i);

		uint32_t sH, sW;
		st.rd.getSwapchainImageExtent(&sW, &sH);
		st.rd.drTraceRays(st.pipeline, sH, sW, i);

		// Present
		st.rd.drSetImageLayoutSimple(st.storageImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i);
		st.rd.drSetSwapchainImageLayoutSimple(i, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i);
		st.rd.drCopyImageToSwapchainImageWithFormatConv(st.storageImage, i, i);
		st.rd.drSetImageLayoutSimple(st.storageImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, i);
		st.rd.drSetSwapchainImageLayoutSimple(i, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, i);
		st.rd.drEndCommandRecording(i);
	}
}

void createStorageImage() {
	st.rd.getSwapchainImageExtent(&st.imgWidth, &st.imgHeight);
	st.rd.createTexture(&st.storageImage, nullptr, nullptr,
		st.imgWidth, st.imgHeight, 4, 0, false, false, AT_IF_SIGNED_FLOAT32, -1, true, AT_IU_RAYTRACING_DEST);
	st.storageImage->toGeneralLayout();

	st.rd.createDescriptorPool(&st.descImg);
	st.rd.addStorageImageArrayToDescriptor({ st.storageImage }, st.descImg, 0, -1);
}

void updateUniformCounter() {
	st.counter += 1;
	float totalLights = st.ldModel.getLightFaces(), totalAreas = st.ldModel.getLightAreas();
	float timex = glfwGetTime();
	st.uniCounter->specifyUniforms(&st.counter, &totalLights, &totalAreas, &timex);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.uniCounter->updateBuffer(i);
	}
}
void mainLoop() {
	updateUniformCounter();
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