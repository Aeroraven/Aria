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
	std::string st(ANTH_SHADER_DIR_HLSL"radixsort\\sort.");
	st += x;
	st += ".hlsl.spv";
	return st;
}

struct CPURadix {
	// Code enclosed in this struct from: 
	// https://www.geeksforgeeks.org/radix-sort/
	int getMax(int a[], int n) {
		int i, max = 0;
		for (i = 0; i < n; i++) {
			if (max < a[i])max = a[i];
		}
		return max;
	}
	void countSort(int a[], int n, int exp) {
		int* output=new int[n];  
		int i, buckets[10] = { 0 };
		int* temp=new int[n];

		for (int i = 0; i < n; i++) {
			buckets[(a[i] / exp) % 10]++;
		}
		for (int i = 1; i < 10; i++) {
			buckets[i] += buckets[i - 1];
		}
		for (int i = n - 1; i >= 0; i--) {
			output[buckets[(a[i] / exp) % 10] - 1] = a[i];
			buckets[(a[i] / exp) % 10]--;
		}
		for (int i = 0; i < n; i++) {
			a[i] = output[i];
		}
	}
	void radixSort(int a[], int n) {
		int exp;
		int max = getMax(a, n);
		for (exp = 1; max / exp > 0; exp *= 10) {
			countSort(a, n, exp);
		}
	}

};

#define DCONST static constexpr const
struct Constants {
	std::string SHADER_GENERATE = getShader("generation.comp");
	std::string SHADER_LOCAL = getShader("local.comp");
	std::string SHADER_GATHER = getShader("gather.comp");
	std::string SHADER_SCATTER = getShader("scatter.comp");

	std::string RESULT_WRITE_SRC = ANTH_ASSET_TEMP_DIR"radixsrc.txt";
	std::string RESULT_WRITE = ANTH_ASSET_TEMP_DIR"radixresult.txt";
	std::string RESULT_WRITE_GT = ANTH_ASSET_TEMP_DIR"radixresultgt.txt";
	std::string RESULT_WRITE_BINS = ANTH_ASSET_TEMP_DIR"radixbins.txt";

	DCONST uint32_t BITS_PER_PASS = 2;
	DCONST uint32_t MAX_BITS = 24;

	DCONST uint32_t BLOCK_SIZE = 1024;
	DCONST uint32_t TOTAL_ELEMENTS = 1024 * 256;
	
	DCONST bool GPU_GENERATE = false;
}sc;
#undef DCONST	

struct Asset {
	// Core
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;

	// Passes
	std::unique_ptr<AnthemComputePassHelper> pGenerate;
	std::unique_ptr<AnthemComputePassHelper> pLocal;
	std::unique_ptr<AnthemComputePassHelper> pGather;
	std::unique_ptr<AnthemComputePassHelper> pScatter;

	// Buffers
	using ShaderBuffer = AnthemShaderStorageBufferImpl<AtBufVecdInt<1>>;
	ShaderBuffer* gSrc;
	ShaderBuffer* gDst;
	ShaderBuffer* gLocalPrefixSum;
	ShaderBuffer* gGlobalPrefixSum;

	// Push Constants
	AnthemUniformBufferImpl<AtUniformVeci<4>, AtUniformVeci<4>, AtUniformVeci<4>>* uniform;
	AnthemPushConstantImpl<AtBufVec4i<1>>* pushConstant;

	// Descriptors
	AnthemDescriptorPool* descSrc;
	AnthemDescriptorPool* descDst;
	AnthemDescriptorPool* descLocalPrefixSum;
	AnthemDescriptorPool* descGlobalPrefixSum;
	AnthemDescriptorPool* descUniform;

	// Command Bufer
	uint32_t sortCmd = 0;
	uint32_t presentCmd[2];

	// Copyback Buffer
	int* copyback;
	int* copybackBins;
	int* cpuData;
	int* sortData;

	// Active Params
	uint32_t curIteration = 0;

	AnthemFence* fence;

}st;

void initialize() {
	// Initialize the renderer
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
}

void createBuffers() {

	st.rd.createDescriptorPool(&st.descSrc);
	st.rd.createDescriptorPool(&st.descDst);
	st.rd.createDescriptorPool(&st.descLocalPrefixSum);
	st.rd.createDescriptorPool(&st.descGlobalPrefixSum);
	st.rd.createDescriptorPool(&st.descUniform);

	st.sortData = new int[sc.TOTAL_ELEMENTS];
	for (auto i : AT_RANGE2(sc.TOTAL_ELEMENTS)) {
		st.sortData[i] = (AnthemLinAlg::randomNumber<float>() * (1 << sc.MAX_BITS));
	}

	std::optional<std::function<void(Asset::ShaderBuffer*)>> cFunc = [&](Asset::ShaderBuffer* b) {
		for (auto i : AT_RANGE2(sc.TOTAL_ELEMENTS)) {
			b->setInput(i, { st.sortData[i] });
		}
	};
	std::optional<std::function<void(Asset::ShaderBuffer*)>> cFunc2 = std::nullopt;

	st.rd.createShaderStorageBuffer(&st.gSrc, sc.TOTAL_ELEMENTS, 0, st.descSrc, cFunc, -1);
	st.rd.createShaderStorageBuffer(&st.gDst, sc.TOTAL_ELEMENTS, 0, st.descDst, cFunc2, -1);
	st.rd.createShaderStorageBuffer(&st.gLocalPrefixSum, sc.TOTAL_ELEMENTS, 0, st.descLocalPrefixSum, cFunc2, -1);
	st.rd.createShaderStorageBuffer(&st.gGlobalPrefixSum, sc.TOTAL_ELEMENTS/sc.BLOCK_SIZE*(1<<sc.BITS_PER_PASS), 0, st.descGlobalPrefixSum, cFunc2, -1);

	st.rd.createUniformBuffer(&st.uniform,0, st.descUniform, -1);

	st.rd.createPushConstant(&st.pushConstant);
	int v[4] = { 0,0,0,0 };
	st.pushConstant->enableShaderStage(AT_APCS_COMPUTE);
	st.pushConstant->setConstant(v);

	//Save Src Data
	std::ofstream file(sc.RESULT_WRITE_SRC);
	for (auto i : AT_RANGE2(sc.TOTAL_ELEMENTS)) {
		file << st.sortData[i] << " ";
		if ((i + 1) % 64 == 0) file << std::endl;
	}
}

void updateUniform() {
	int workGroupSize[4] = { sc.BLOCK_SIZE, 1, 1, 1 };
	int parallelBits[4] = { st.curIteration, sc.BITS_PER_PASS, 0, 0 };
	int sequenceLength[4] = { sc.TOTAL_ELEMENTS, 0, 0, 0 };
		
	st.uniform->specifyUniforms(workGroupSize, parallelBits, sequenceLength);
	for (auto i : AT_RANGE2(2)) {
		st.uniform->updateBuffer(i);
	}
}

void buildPipelines() {
	st.pGenerate = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pGenerate->shaderPath.computeShader = sc.SHADER_GENERATE;
	st.pGenerate->setDescriptorLayouts({
		{st.descSrc,AT_ACDS_SHADER_STORAGE_BUFFER,0}
	});
	st.pGenerate->buildComputePipeline();

	st.pLocal = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pLocal->shaderPath.computeShader = sc.SHADER_LOCAL;
	st.pLocal->pushConstants.push_back(st.pushConstant);
	st.pLocal->setDescriptorLayouts({
		{st.descSrc,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descGlobalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descLocalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.pLocal->buildComputePipeline();

	st.pGather = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pGather->shaderPath.computeShader = sc.SHADER_GATHER;
	st.pGather->pushConstants.push_back(st.pushConstant);
	st.pGather->setDescriptorLayouts({
		{st.descGlobalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.pGather->buildComputePipeline();

	st.pScatter = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pScatter->shaderPath.computeShader = sc.SHADER_SCATTER;
	st.pScatter->pushConstants.push_back(st.pushConstant);
	st.pScatter->setDescriptorLayouts({
		{st.descSrc,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descDst,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descGlobalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descLocalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.pScatter->buildComputePipeline();
}

void buildCommand() {
	st.rd.drAllocateCommandBuffer(&st.sortCmd);
	st.rd.drStartCommandRecording(st.sortCmd);

	uint32_t totalIterations = sc.MAX_BITS / sc.BITS_PER_PASS;

	//Generate
	if (sc.GPU_GENERATE) {
		st.rd.drBindComputePipeline(st.pGenerate->pipeline, st.sortCmd);
		st.rd.drBindDescriptorSetCustomizedCompute(st.pGenerate->descLayout[0], st.pGenerate->pipeline, st.sortCmd);
		st.rd.drComputeDispatch(st.sortCmd, sc.TOTAL_ELEMENTS / sc.BLOCK_SIZE, 1, 1);

		st.gSrc->RWBarrier(st.sortCmd, 0);
	}


	for (auto it : AT_RANGE2(totalIterations)) {
		auto dsrc=(it%2== 0) ? st.descSrc : st.descDst;
		auto ddst=(it%2== 0) ? st.descDst : st.descSrc;
		auto bsrc=(it%2== 0) ? st.gSrc : st.gDst;
		auto bdst=(it%2== 0) ? st.gDst : st.gSrc;

		int v[4] = { it,it,0,0 };
		st.pushConstant->setConstant(v);

		//Local
		st.rd.drBindComputePipeline(st.pLocal->pipeline, st.sortCmd);
		st.rd.drBindDescriptorSetCustomizedCompute({
			{dsrc,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descGlobalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descLocalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0} 
		}, st.pLocal->pipeline, st.sortCmd);
		st.rd.drPushConstantsCompute(st.pushConstant,st.pLocal->pipeline,st.sortCmd);
		st.rd.drComputeDispatch(st.sortCmd, sc.TOTAL_ELEMENTS / sc.BLOCK_SIZE, 1, 1);

		st.gGlobalPrefixSum->RWBarrier(st.sortCmd, 0);

		//Gather
		st.rd.drBindComputePipeline(st.pGather->pipeline, st.sortCmd);
		st.rd.drBindDescriptorSetCustomizedCompute({
			{st.descGlobalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0}
		}, st.pGather->pipeline, st.sortCmd);
		st.rd.drPushConstantsCompute(st.pushConstant, st.pGather->pipeline, st.sortCmd);
		st.rd.drComputeDispatch(st.sortCmd, 1, 1, 1);

		st.gGlobalPrefixSum->RWBarrier(st.sortCmd, 0);
		st.gLocalPrefixSum->RWBarrier(st.sortCmd, 0);
		bsrc->RWBarrier(st.sortCmd, 0);

		//Scatter
		st.rd.drBindComputePipeline(st.pScatter->pipeline, st.sortCmd);
		st.rd.drBindDescriptorSetCustomizedCompute({
			{dsrc,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{ddst,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descGlobalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descLocalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0}
		}, st.pScatter->pipeline, st.sortCmd);
		st.rd.drPushConstantsCompute(st.pushConstant, st.pScatter->pipeline, st.sortCmd);
		st.rd.drComputeDispatch(st.sortCmd, sc.TOTAL_ELEMENTS / sc.BLOCK_SIZE, 1, 1);

		bdst->RWBarrier(st.sortCmd, 0);
	}
	
	st.rd.drEndCommandRecording(st.sortCmd);
	st.rd.createFence(&st.fence);
	st.fence->resetFence();

	for (auto i : AT_RANGE2(2)) {
		st.rd.drAllocateCommandBuffer(&st.presentCmd[i]);
		st.rd.drStartCommandRecording(st.presentCmd[i]);
		st.rd.drSetSwapchainImageLayoutSimple(0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, st.presentCmd[i]);
		st.rd.drSetSwapchainImageLayoutSimple(1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, st.presentCmd[i]);

		st.rd.drEndCommandRecording(st.presentCmd[i]);
	}

}
void copyBackToFile() {

	std::ofstream file(sc.RESULT_WRITE);
	for (auto i : AT_RANGE2(sc.TOTAL_ELEMENTS)) {
		file << st.copyback[i] << " ";
		if ((i+1) % 64 == 0) file << std::endl;
	}

	st.copybackBins = new int[sc.TOTAL_ELEMENTS];
	st.gGlobalPrefixSum->copyDataBack(0, st.copybackBins);
	std::ofstream file2(sc.RESULT_WRITE_BINS);
	for (auto i : AT_RANGE2(sc.TOTAL_ELEMENTS)) {
		file2 << st.copybackBins[i] << " ";
		if ((i + 1) % 64 == 0) file2 << std::endl;
	}
}

void runCommand() {
	//Record time
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	st.rd.drSubmitCommandBufferCompQueueGeneralA(st.sortCmd, {}, {}, st.fence);
	st.fence->waitAndReset();

	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
	ANTH_LOGI("Time taken for GPU Radix Sort: ");
	ANTH_LOGI(" - Device Local:", time_span.count(), " seconds");
	auto sf = time_span.count();
	st.copyback = new int[sc.TOTAL_ELEMENTS];

	start = std::chrono::high_resolution_clock::now();
	st.gSrc->copyDataBack(0, st.copyback);
	end = std::chrono::high_resolution_clock::now();
	time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

	auto totalTime = time_span.count() + sf;
	ANTH_LOGI(" - Copyback: ", time_span.count(), " seconds");
	ANTH_LOGI(" - Total: ", totalTime, " seconds");
	ANTH_LOGI("");

	//CPU
	st.cpuData = new int[sc.TOTAL_ELEMENTS];
	for (auto i : AT_RANGE2(sc.TOTAL_ELEMENTS)) {
		st.cpuData[i] = st.sortData[i];
	}
	start = std::chrono::high_resolution_clock::now();
	std::sort(st.cpuData, st.cpuData + sc.TOTAL_ELEMENTS);
	end = std::chrono::high_resolution_clock::now();
	time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
	ANTH_LOGI("Time taken for CPU std::sort :");
	ANTH_LOGI(" - Total:", time_span.count(), " seconds");
	ANTH_LOGI("");

	// Save Ground Truth
	std::ofstream file(sc.RESULT_WRITE_GT);
	for (auto i : AT_RANGE2(sc.TOTAL_ELEMENTS)) {
		file << st.cpuData[i] << " ";
		if ((i + 1) % 64 == 0) file << std::endl;
	}



	//CPU Radix
	for (auto i : AT_RANGE2(sc.TOTAL_ELEMENTS)) {
		st.cpuData[i] = st.sortData[i];
	}
	start = std::chrono::high_resolution_clock::now();
	CPURadix cr;
	cr.radixSort(st.cpuData, sc.TOTAL_ELEMENTS);
	end = std::chrono::high_resolution_clock::now();
	time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
	ANTH_LOGI("Time taken for CPU Radix Sort: ");	
	ANTH_LOGI(" - Total:", time_span.count(), " seconds");
	ANTH_LOGI("");

	
}

int main() {
	initialize();
	createBuffers();
	buildPipelines();

	st.rd.registerPipelineSubComponents();

	buildCommand();
	updateUniform();
	runCommand();
	copyBackToFile();
	st.rd.finalize();
	return 0;
}