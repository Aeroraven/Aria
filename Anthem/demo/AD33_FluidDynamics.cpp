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
	DCONST uint32_t GRID_X = 1024;
	DCONST uint32_t GRID_Y = 1024;
	DCONST uint32_t THREAD_X = 16;
	DCONST uint32_t THREAD_Y = 16;
	DCONST float DIFFUSION_COEF = 1.0f;
	DCONST float TIMESTEP = 0.001f;
	DCONST uint32_t JACOBI_ITERS = 80;
	DCONST std::array<float, 4> CLEAR_COLOR = { 1,0,0,1 };

	std::string SHADER_ADVECTION = getShader("advection.comp");
	std::string SHADER_DIFFUSION_SOLVER = getShader("diffusion.comp");
	std::string SHADER_PRESSURE_SOLVER = getShader("pressure.comp");
	std::string SHADER_SUBTRACT = getShader("subtract.comp");
	std::string SHADER_INIT = getShader("init.comp");

	std::string SHADER_VISVS = getShader("vis.vert");
	std::string SHADER_VISFS = getShader("vis.frag");


}sc;
#undef DCONST

struct Assets {
	// Core
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;

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
	std::unique_ptr<AnthemComputePassHelper> pAdvection;
	std::unique_ptr<AnthemComputePassHelper> pDiffusion;
	std::unique_ptr<AnthemComputePassHelper> pPressureSolver;
	std::unique_ptr<AnthemComputePassHelper> pDivSubtraction;
	std::unique_ptr<AnthemComputePassHelper> pInit;
	std::unique_ptr<AnthemPassHelper> pVisualization;

	// Visualization
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>>* vx;
	AnthemIndexBuffer* ix;

	// Executor
	std::unique_ptr<AnthemSequentialCommand> execSeq[2];

	// Uniforms
	AnthemUniformBufferImpl<
		AtUniformVecf<4>,
		AtUniformVeci<4>
	>* uniform;
	AnthemDescriptorPool* descUniform;

	//Command Buffer
	uint32_t compCmd[2];
	uint32_t initCmd[2];
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

	st.pVisualization = std::make_unique<AnthemPassHelper>(&st.rd,2);
	st.pVisualization->shaderPath.vertexShader = sc.SHADER_VISVS;
	st.pVisualization->shaderPath.fragmentShader = sc.SHADER_VISFS;
	st.pVisualization->vxLayout = st.vx;
	st.pVisualization->passOpt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	st.pVisualization->pipeOpt.blendPreset = { AT_ABP_NO_BLEND };
	st.pVisualization->setDescriptorLayouts({
		{st.descDyeVis[0], AT_ACDS_SAMPLER,0},
		}, 0);
	st.pVisualization->setDescriptorLayouts({
		{st.descDyeVis[1], AT_ACDS_SAMPLER,0},
		}, 1);
	st.pVisualization->buildGraphicsPipeline();
}
void recordCommandBuffer() {
	for (auto i : AT_RANGE2(2)) {
		auto c = st.compCmd[i];
		st.rd.drStartCommandRecording(c);
		recordAdvection(c, st.descVelocity[1 - i], st.descVelocity[1 - i], st.descVelocity[i]);
		clearImage(c, st.velocityFieldTemp[0]);
		clearImage(c, st.velocityFieldTemp[1]);
		for (auto j : AT_RANGE2(sc.JACOBI_ITERS)) {
			recordDiffusion(c, st.descVelocity[i], st.descVelocityTemp[1 - j % 2], st.descVelocityTemp[j % 2]);
		}
		clearImage(c, st.pressureFieldTemp[0]);
		clearImage(c, st.pressureFieldTemp[1]);
		for (auto j : AT_RANGE2(sc.JACOBI_ITERS)) {
			recorePressureSolver(c, st.descVelocityTemp[1 - sc.JACOBI_ITERS % 2], st.descPressureTemp[1 - j % 2], st.descPressureTemp[j % 2]);
		}
		recordSubtraction(c, st.descVelocityTemp[1 - sc.JACOBI_ITERS % 2], st.descPressureTemp[1 - sc.JACOBI_ITERS % 2], st.descVelocity[i]);
		recordAdvection(c, st.descVelocity[i], st.descDye[1 - i], st.descDye[i]);
		st.rd.drEndCommandRecording(c);
	}
	st.pVisualization->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.vx, x);
		st.rd.drBindIndexBuffer(st.ix, x);
		st.rd.drDraw(6, x);
	});
	
	for (auto i : AT_RANGE2(2)) {
		st.execSeq[i] = std::make_unique<AnthemSequentialCommand>(&st.rd);
		st.execSeq[i]->setSequence({
			//{st.compCmd[i],ATC_ASCE_COMPUTE},
			{st.pVisualization->getCommandIndex(i),ATC_ASCE_GRAPHICS}
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

void updateUniform() {
	float data1[4] = { sc.DIFFUSION_COEF,sc.TIMESTEP,0,0 };
	int data2[4] = { sc.GRID_X,sc.GRID_Y,0,0 };
	st.uniform->specifyUniforms(data1, data2);
	for (auto i : AT_RANGE2(2)) {
		st.uniform->updateBuffer(i);
	}
}

void drawLoop() {
	static int cur = 0;
	uint32_t imgIdx;
	updateUniform();
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.execSeq[cur]->executeCommandToStage(imgIdx, false, false, st.pVisualization->getSwapchainBuffer());
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}


int main() {
	initialize();
	createComputeDependency();
	createFields();
	createComputePipelines();
	createGraphicsPipeline();

	st.rd.registerPipelineSubComponents();
	recordCommandBuffer();
	initFluid();

	st.rd.setDrawFunction(drawLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();

	return 0;
}