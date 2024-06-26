#include "../include/components/camera/AnthemCamera.h"
#include "../include/components/performance/AnthemFrameRateMeter.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"
#include "../include/core/drawing/buffer/impl/AnthemVertexBufferImpl.h"
#include "../include/core/drawing/image/AnthemImage.h"
#include "../include/core/drawing/buffer/impl/AnthemShaderStorageBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemPushConstantImpl.h"
#include "../include/components/passhelper/AnthemPassHelper.h"
#include "../include/components/passhelper/AnthemComputePassHelper.h"
#include "../include/components/passhelper/AnthemSequentialCommand.h"
#include "../include/components/postprocessing/AnthemPostIdentity.h"
#include "../include/components/postprocessing/AnthemFXAA.h"
#include "../include/components/postprocessing/AnthemBloom.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Camera;
using namespace Anthem::Components::PassHelper;
using namespace Anthem::Components::Postprocessing;
using namespace Anthem::Core;

constexpr inline std::string getShader(std::string x) {
	std::string st(ANTH_SHADER_DIR_HLSL"fluid\\fl.");
	st += x;
	st += ".hlsl.spv";
	return st;
}


#define DCONST constexpr static const
struct Parameters {
	DCONST uint32_t GRID_X = 912;
	DCONST uint32_t GRID_Y = 512;
	DCONST uint32_t THREAD_X = 16;
	DCONST uint32_t THREAD_Y = 16;

	DCONST float TIMESTEP = 0.2f;
	DCONST uint32_t JACOBI_ITERS = 60;
	DCONST std::array<float, 4> CLEAR_COLOR = { 1,0,0,1 };
	DCONST uint32_t CLEAR_PRESSURE = 20.0f;
	

	float VISCOSITY_COEF = -5.0;
	float SPLAT_RADIUS = 80.0f;
	float DYE_DECAY = 0.985f;
	float HUE_CHANGE_RATE = 0.9;

	std::string SHADER_ADVECTION = getShader("advection.comp");
	std::string SHADER_DIFFUSION_SOLVER = getShader("diffusion.comp");
	std::string SHADER_PRESSURE_SOLVER = getShader("pressure.comp");
	std::string SHADER_SUBTRACT = getShader("subtract.comp");
	std::string SHADER_INIT = getShader("init.comp");
	std::string SHADER_SPLAT = getShader("splat.comp");

	std::string SHADER_VISVS = getShader("vis.vert");
	std::string SHADER_VISFS = getShader("vis.frag");


}sc;
#undef DCONST

struct Assets {
	// Core
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemFrameRateMeter frm = AnthemFrameRateMeter(10);

	// Fluid Attributes
	AnthemImage* dyeField[2];
	AnthemImage* velocityField[2];
	AnthemImage* pressureField[2];
	AnthemImage* velocityFieldTemp[2];
	AnthemImage* pressureFieldTemp[2];

	AnthemDescriptorPool* descDye[2];
	AnthemDescriptorPool* descDyeVis[2];
	AnthemDescriptorPool* descVelocity[2];
	AnthemDescriptorPool* descPressure[2];
	AnthemDescriptorPool* descVelocityTemp[2];
	AnthemDescriptorPool* descPressureTemp[2];

	// Pipelines
	AnthemImage* renderTarget;
	AnthemDescriptorPool* descTarget;

	std::unique_ptr<AnthemComputePassHelper> pSplat;
	std::unique_ptr<AnthemComputePassHelper> pAdvection;
	std::unique_ptr<AnthemComputePassHelper> pDiffusion;
	std::unique_ptr<AnthemComputePassHelper> pPressureSolver;
	std::unique_ptr<AnthemComputePassHelper> pDivSubtraction;
	std::unique_ptr<AnthemComputePassHelper> pInit;
	std::unique_ptr<AnthemPassHelper> pVisualization;
	std::unique_ptr<AnthemBloom> pBloom;


	// Visualization
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>>* vx;
	AnthemIndexBuffer* ix;

	// Executor
	std::unique_ptr<AnthemSequentialCommand> execSeq[2];

	// Uniforms
	AnthemUniformBufferImpl<
		AtUniformVecf<4>,	//Viscosity, timestep, splatRadius, 0
		AtUniformVeci<4>,
		AtUniformVecf<4>,
		AtUniformVecf<4>,	//Window
		AtUniformVecf<4>	//Decay
	>* uniform;
	AnthemDescriptorPool* descUniform;

	// Command Buffer
	uint32_t compCmd[2];
	uint32_t initCmd[2];

	// Handlers
	int lastX = 0;
	int lastY = 0;
	int movDx = 0;
	int movDy = 0;
	int splatX = 0;
	int splatY = 0;
	int enableMov = 0;
	bool splatEnable = false;
	std::function<void(int, int, int)> mouseClickHandler;
	std::function<void(double, double)> mouseMoveHandler;
}st;

void initialize() {
	st.cfg.demoName = "33. Compute Fluid Dynamics";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
}

void createComputeDependency() {
	st.rd.drAllocateCommandBuffer(&st.compCmd[0]);
	st.rd.drAllocateCommandBuffer(&st.compCmd[1]);
	st.rd.createDescriptorPool(&st.descUniform);
	st.rd.createUniformBuffer(&st.uniform, 0, st.descUniform, -1);
}

void createFields() {
	for (auto i : AT_RANGE2(2)) {
		st.rd.createDescriptorPool(&st.descDye[i]);
		st.rd.createDescriptorPool(&st.descDyeVis[i]);
		st.rd.createDescriptorPool(&st.descVelocity[i]);
		st.rd.createDescriptorPool(&st.descPressure[i]);
		st.rd.createDescriptorPool(&st.descVelocityTemp[i]);
		st.rd.createDescriptorPool(&st.descPressureTemp[i]);

		st.rd.createTexture(&st.dyeField[i], nullptr, nullptr, sc.GRID_X, sc.GRID_Y, 4, 0, false, false, AT_IF_SIGNED_FLOAT32, 0, true, AT_IU_COMPUTE_OUTPUT);
		st.rd.createTexture(&st.velocityField[i], nullptr, nullptr, sc.GRID_X, sc.GRID_Y, 4, 0, false, false, AT_IF_SIGNED_FLOAT32, 0, true, AT_IU_COMPUTE_OUTPUT);
		st.rd.createTexture(&st.pressureField[i], nullptr, nullptr, sc.GRID_X, sc.GRID_Y, 4, 0, false, false, AT_IF_SIGNED_FLOAT32, 0, true, AT_IU_COMPUTE_OUTPUT);
		st.rd.createTexture(&st.velocityFieldTemp[i], nullptr, nullptr, sc.GRID_X, sc.GRID_Y, 4, 0, false, false, AT_IF_SIGNED_FLOAT32, 0, true, AT_IU_COMPUTE_OUTPUT);
		st.rd.createTexture(&st.pressureFieldTemp[i], nullptr, nullptr, sc.GRID_X, sc.GRID_Y, 4, 0, false, false, AT_IF_SIGNED_FLOAT32, 0, true, AT_IU_COMPUTE_OUTPUT);

		st.dyeField[i]->toGeneralLayout();
		st.velocityField[i]->toGeneralLayout();
		st.pressureField[i]->toGeneralLayout();
		st.velocityFieldTemp[i]->toGeneralLayout();
		st.pressureFieldTemp[i]->toGeneralLayout();

		st.rd.addStorageImageArrayToDescriptor({ st.dyeField[i] }, st.descDye[i], 0, -1);
		st.rd.addStorageImageArrayToDescriptor({ st.velocityField[i] }, st.descVelocity[i], 0, -1);
		st.rd.addStorageImageArrayToDescriptor({ st.pressureField[i] }, st.descPressure[i], 0, -1);
		st.rd.addStorageImageArrayToDescriptor({ st.velocityFieldTemp[i] }, st.descVelocityTemp[i], 0, -1);
		st.rd.addStorageImageArrayToDescriptor({ st.pressureFieldTemp[i] }, st.descPressureTemp[i], 0, -1);

		st.rd.addSamplerArrayToDescriptor({ st.dyeField[i] }, st.descDyeVis[i], 0, -1);
	}
}

void clearImage(uint32_t commandBuffer, AnthemImage* image) {
	st.rd.drClearColorImageFloat(image, sc.CLEAR_COLOR, VK_IMAGE_LAYOUT_GENERAL, commandBuffer);
}

void clearPressure(uint32_t commandBuffer, AnthemImage* image) {
	st.rd.drClearColorImageFloat(image, {sc.CLEAR_PRESSURE*1.0f,0,0,1}, VK_IMAGE_LAYOUT_GENERAL, commandBuffer);
}

void recordSplat(uint32_t commandBuffer,AnthemDescriptorPool* outVelocity, AnthemDescriptorPool* outDye) {
	st.rd.drBindComputePipeline(st.pSplat->pipeline, commandBuffer);
	st.rd.drBindDescriptorSetCustomizedCompute({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{outVelocity, AT_ACDS_STORAGE_IMAGE,0},
		{outDye, AT_ACDS_STORAGE_IMAGE,0},
	}, st.pSplat->pipeline, commandBuffer);
	st.rd.drComputeDispatch(commandBuffer, st.pSplat->workGroupSize[0], st.pSplat->workGroupSize[1], st.pSplat->workGroupSize[2]);

}
void recordAdvection(uint32_t commandBuffer, AnthemDescriptorPool* lastVelocity, 
	AnthemDescriptorPool* qtyToAdvect, AnthemDescriptorPool* qtyAdvected) {
	st.rd.drBindComputePipeline(st.pAdvection->pipeline, commandBuffer);
	st.rd.drBindDescriptorSetCustomizedCompute({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{lastVelocity, AT_ACDS_STORAGE_IMAGE,0},
		{qtyToAdvect, AT_ACDS_STORAGE_IMAGE,0},
		{qtyAdvected, AT_ACDS_STORAGE_IMAGE,0},
	}, st.pAdvection->pipeline, commandBuffer);
	st.rd.drComputeDispatch(commandBuffer, st.pAdvection->workGroupSize[0], st.pAdvection->workGroupSize[1], st.pAdvection->workGroupSize[2]);
}

void recordDiffusion(uint32_t commandBuffer, AnthemDescriptorPool* prevTimeVelocity,
	AnthemDescriptorPool* prevStepVelocity, AnthemDescriptorPool* nextStepVelocity) {
	st.rd.drBindComputePipeline(st.pDiffusion->pipeline, commandBuffer);
	st.rd.drBindDescriptorSetCustomizedCompute({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{prevTimeVelocity, AT_ACDS_STORAGE_IMAGE,0},
		{prevStepVelocity, AT_ACDS_STORAGE_IMAGE,0},
		{nextStepVelocity, AT_ACDS_STORAGE_IMAGE,0},
		}, st.pDiffusion->pipeline, commandBuffer);
	st.rd.drComputeDispatch(commandBuffer, st.pDiffusion->workGroupSize[0], st.pDiffusion->workGroupSize[1], st.pDiffusion->workGroupSize[2]);
}

void recorePressureSolver(uint32_t commandBuffer, AnthemDescriptorPool* curVelocity,
	AnthemDescriptorPool* prevStepPressure, AnthemDescriptorPool* nextStepPressure) {
	st.rd.drBindComputePipeline(st.pPressureSolver->pipeline, commandBuffer);
	st.rd.drBindDescriptorSetCustomizedCompute({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{curVelocity, AT_ACDS_STORAGE_IMAGE,0},
		{prevStepPressure, AT_ACDS_STORAGE_IMAGE,0},
		{nextStepPressure, AT_ACDS_STORAGE_IMAGE,0},
		}, st.pPressureSolver->pipeline, commandBuffer);
	st.rd.drComputeDispatch(commandBuffer, st.pPressureSolver->workGroupSize[0], st.pPressureSolver->workGroupSize[1], st.pPressureSolver->workGroupSize[2]);
}

void recordSubtraction(uint32_t commandBuffer, AnthemDescriptorPool* curVelocity,
	AnthemDescriptorPool* curPressure, AnthemDescriptorPool* outVelocity) {
	st.rd.drBindComputePipeline(st.pDivSubtraction->pipeline, commandBuffer);
	st.rd.drBindDescriptorSetCustomizedCompute({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{curVelocity, AT_ACDS_STORAGE_IMAGE,0},
		{curPressure, AT_ACDS_STORAGE_IMAGE,0},
		{outVelocity, AT_ACDS_STORAGE_IMAGE,0},
	}, st.pDivSubtraction->pipeline, commandBuffer);
	st.rd.drComputeDispatch(commandBuffer, st.pDivSubtraction->workGroupSize[0], st.pDivSubtraction->workGroupSize[1], st.pDivSubtraction->workGroupSize[2]);
}

void createComputePipelines() {
	st.pSplat = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pSplat->workGroupSize[0] = sc.GRID_X / sc.THREAD_X;
	st.pSplat->workGroupSize[1] = sc.GRID_Y / sc.THREAD_Y;
	st.pSplat->workGroupSize[2] = 1;
	st.pSplat->shaderPath.computeShader = sc.SHADER_SPLAT;
	st.pSplat->setDescriptorLayouts({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		});
	st.pSplat->buildComputePipeline();

	st.pInit = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pInit->workGroupSize[0] = sc.GRID_X / sc.THREAD_X;
	st.pInit->workGroupSize[1] = sc.GRID_Y / sc.THREAD_Y;
	st.pInit->workGroupSize[2] = 1;
	st.pInit->shaderPath.computeShader = sc.SHADER_INIT;
	st.pInit->setDescriptorLayouts({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		});
	st.pInit->buildComputePipeline();

	st.pAdvection = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pAdvection->workGroupSize[0] = sc.GRID_X / sc.THREAD_X;
	st.pAdvection->workGroupSize[1] = sc.GRID_Y / sc.THREAD_Y;
	st.pAdvection->workGroupSize[2] = 1;
	st.pAdvection->shaderPath.computeShader = sc.SHADER_ADVECTION;
	st.pAdvection->setDescriptorLayouts({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0}
		});
	st.pAdvection->buildComputePipeline();

	st.pDiffusion = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pDiffusion->workGroupSize[0] = sc.GRID_X / sc.THREAD_X;
	st.pDiffusion->workGroupSize[1] = sc.GRID_Y / sc.THREAD_Y;
	st.pDiffusion->workGroupSize[2] = 1;
	st.pDiffusion->shaderPath.computeShader = sc.SHADER_DIFFUSION_SOLVER;
	st.pDiffusion->setDescriptorLayouts({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0}
	});
	st.pDiffusion->buildComputePipeline();
	
	st.pPressureSolver = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pPressureSolver->workGroupSize[0] = sc.GRID_X / sc.THREAD_X;
	st.pPressureSolver->workGroupSize[1] = sc.GRID_Y / sc.THREAD_Y;
	st.pPressureSolver->workGroupSize[2] = 1;
	st.pPressureSolver->shaderPath.computeShader = sc.SHADER_PRESSURE_SOLVER;
	st.pPressureSolver->setDescriptorLayouts({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0}
		});
	st.pPressureSolver->buildComputePipeline();

	st.pDivSubtraction = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pDivSubtraction->workGroupSize[0] = sc.GRID_X / sc.THREAD_X;
	st.pDivSubtraction->workGroupSize[1] = sc.GRID_Y / sc.THREAD_Y;
	st.pDivSubtraction->workGroupSize[2] = 1;
	st.pDivSubtraction->shaderPath.computeShader = sc.SHADER_SUBTRACT;
	st.pDivSubtraction->setDescriptorLayouts({
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0},
		{st.descDye[0], AT_ACDS_STORAGE_IMAGE,0}
		});
	st.pDivSubtraction->buildComputePipeline();
}

void createGraphicsPipeline() {
	st.rd.createVertexBuffer(&st.vx);
	st.rd.createIndexBuffer(&st.ix);

	st.vx->setTotalVertices(4);
	st.vx->insertData(0, { -1,-1,0,1 }, { 0,0,0,0 });
	st.vx->insertData(1, { 1,-1,0,1 }, { 1,0,0,0 });
	st.vx->insertData(2, { 1,1,0,1 }, { 1,1,0,0 });
	st.vx->insertData(3, { -1,1,0,1 }, { 0,1,0,0 });

	st.ix->setIndices({ 0,1,2,2,3,0 });

	st.rd.createDescriptorPool(&st.descTarget);
	st.rd.createColorAttachmentImage(&st.renderTarget, st.descTarget, 0, AT_IF_SIGNED_FLOAT32, false, -1, false);

	st.pVisualization = std::make_unique<AnthemPassHelper>(&st.rd,2);
	st.pVisualization->shaderPath.vertexShader = sc.SHADER_VISVS;
	st.pVisualization->shaderPath.fragmentShader = sc.SHADER_VISFS;
	st.pVisualization->vxLayout = st.vx;
	st.pVisualization->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.pVisualization->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.pVisualization->setRenderTargets({ st.renderTarget });
	st.pVisualization->pipeOpt.blendPreset = { AT_ABP_NO_BLEND };
	st.pVisualization->setDescriptorLayouts({
		{st.descDyeVis[0], AT_ACDS_SAMPLER,0},
		}, 0);
	st.pVisualization->setDescriptorLayouts({
		{st.descDyeVis[1], AT_ACDS_SAMPLER,0},
		}, 1);
	st.pVisualization->buildGraphicsPipeline();

	st.pBloom = std::make_unique<AnthemBloom>(&st.rd, 2, 5, st.cfg.appcfgResolutionWidth, st.cfg.appcfgResolutionHeight);
	st.pBloom->setSrcImage(st.descTarget);
	st.pBloom->prepare(false);
}
void recordCommandBuffer() {
	for (auto i : AT_RANGE2(2)) {
		auto c = st.compCmd[i];
		st.rd.drStartCommandRecording(c);
		recordAdvection(c, st.descVelocity[1 - i], st.descVelocity[1 - i], st.descVelocity[i]);
		clearImage(c, st.velocityFieldTemp[0]);
		clearImage(c, st.velocityFieldTemp[1]);
		for (auto j : AT_RANGE2(sc.JACOBI_ITERS)) {
			recordDiffusion(c, st.descVelocity[i], j==0? st.descVelocity[i]: st.descVelocityTemp[1 - j % 2], st.descVelocityTemp[j % 2]);
		}
		clearPressure(c, st.pressureFieldTemp[0]);
		clearPressure(c, st.pressureFieldTemp[1]);
		for (auto j : AT_RANGE2(sc.JACOBI_ITERS)) {
			recorePressureSolver(c, st.descVelocityTemp[1 - sc.JACOBI_ITERS % 2], st.descPressureTemp[1 - j % 2], st.descPressureTemp[j % 2]);
		}
		recordSubtraction(c, st.descVelocityTemp[1 - sc.JACOBI_ITERS % 2], st.descPressureTemp[1 - sc.JACOBI_ITERS % 2], st.descVelocity[i]);
		recordAdvection(c, st.descVelocity[i], st.descDye[1 - i], st.descDye[i]);
		
		recordSplat(c, st.descVelocity[i], st.descDye[i]);
		st.rd.drEndCommandRecording(c);
	}
	st.pVisualization->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.vx, x);
		st.rd.drBindIndexBuffer(st.ix, x);
		st.rd.drDraw(6, x);
	});
	st.pBloom->recordCommand();
	
	for (auto i : AT_RANGE2(2)) {
		st.execSeq[i] = std::make_unique<AnthemSequentialCommand>(&st.rd);
		st.execSeq[i]->setSequence({
			{st.compCmd[i],ATC_ASCE_COMPUTE},
			{st.pVisualization->getCommandIndex(i),ATC_ASCE_GRAPHICS},
			{st.pBloom->getCommandIdx(i),ATC_ASCE_GRAPHICS}
		});
	}
}

void initFluid() {
	for (auto i : AT_RANGE2(2)) {
		uint32_t cmd;
		AnthemFence* fence;
		st.rd.createFence(&fence);
		st.rd.drAllocateCommandBuffer(&cmd);
		st.rd.drStartCommandRecording(cmd);
		st.rd.drBindComputePipeline(st.pInit->pipeline, cmd);
		st.rd.drBindDescriptorSetCustomizedCompute({
			{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0},
			{st.descDye[i], AT_ACDS_STORAGE_IMAGE,0},
			{st.descVelocity[i], AT_ACDS_STORAGE_IMAGE,0},
			}, st.pInit->pipeline, cmd);
		st.rd.drComputeDispatch(cmd, st.pInit->workGroupSize[0], st.pInit->workGroupSize[1], st.pInit->workGroupSize[2]);
		st.rd.drEndCommandRecording(cmd);

		fence->resetFence();
		st.rd.drSubmitCommandBufferCompQueueGeneralA(cmd, {}, {}, fence);
		fence->waitAndReset();
	}
}

void registerSplatHooks() {
	st.mouseClickHandler =  [&](int button, int action, int mods) {
		if (button == 0 && action == 1) {
			st.splatEnable = true;
		}
		if (button == 0 && action == 0) {
			st.splatEnable = false;
		}
	};
	st.mouseMoveHandler = [&](double x, double y) {
		if (st.splatEnable) {
			st.movDx = x - st.lastX;
			st.movDy = y - st.lastY;
			st.enableMov = 1;
		}
		else {
			st.enableMov = 0;
		}
		st.splatX = x;
		st.splatY = y;
		st.lastX = x;
		st.lastY = y;
	};
	st.rd.ctSetMouseController(st.mouseClickHandler);
	st.rd.ctSetMouseMoveController(st.mouseMoveHandler);
}

void prepareImguiFrame() {
	st.frm.record();
	std::stringstream ss;
	ss << "FPS:";
	ss << st.frm.getFrameRate();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Control Panel");
	ImGui::Text(ss.str().c_str());
	ImGui::SliderFloat("Splat Radius", &sc.SPLAT_RADIUS, 0.0f, 200.0f, "%.4f");
	ImGui::SliderFloat("Log Viscosity", &sc.VISCOSITY_COEF, 0.0f, 200.0f, "%.4f");
	ImGui::SliderFloat("Decay", &sc.DYE_DECAY, 0.0f, 1.0f, "%.5f");
	ImGui::SliderFloat("Hue Freq", &sc.HUE_CHANGE_RATE, 0.0f, 5.0f, "%.5f");
	ImGui::End();
}

void updateUniform() {
	int rdW, rdH;
	st.rd.exGetWindowSize(rdH, rdW);
	float data1[4] = { std::pow(10.0,sc.VISCOSITY_COEF),sc.TIMESTEP,sc.SPLAT_RADIUS,st.enableMov };
	int data2[4] = { sc.GRID_X,sc.GRID_Y,0,0 };
	float splatData[4] = { st.movDx,st.movDy,st.splatX,st.splatY };
	float windowData[4] = { rdW,rdH,0,0 };
	float decay[4] = { sc.DYE_DECAY,glfwGetTime() * sc.HUE_CHANGE_RATE,0 };
	st.uniform->specifyUniforms(data1, data2, splatData, windowData,decay);
	for (auto i : AT_RANGE2(2)) {
		st.uniform->updateBuffer(i);
	}
}

void setupImgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.FontGlobalScale = 1.8;
	ImGui::StyleColorsDark();
	st.rd.exInitImGui();
}

void drawLoop() {
	static int cur = 0;
	uint32_t imgIdx;
	updateUniform();
	st.rd.drPrepareFrame(cur, &imgIdx);
	prepareImguiFrame();
	st.execSeq[cur]->executeCommandToStage(imgIdx, false, true, st.pBloom->getSwapchainFb());
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}


int main() {
	initialize();
	setupImgui();
	registerSplatHooks();
	createComputeDependency();
	createFields();
	createComputePipelines();
	createGraphicsPipeline();

	st.rd.registerPipelineSubComponents();
	recordCommandBuffer();
	initFluid();

	st.rd.setDrawFunction(drawLoop);
	st.rd.startDrawLoopDemo();
	st.rd.exDestroyImgui();
	st.rd.finalize();

	return 0;
}