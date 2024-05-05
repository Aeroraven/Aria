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

#define DCONST static constexpr const
struct Constants {
	std::string SHADER_GENERATE = getShader("generation.comp");
	std::string SHADER_LOCAL = getShader("local.comp");
	std::string SHADER_GATHER = getShader("gather.comp");
	std::string SHADER_SCATTER = getShader("scatter.comp");

	DCONST uint32_t BITS_PER_PASS = 4;
	DCONST uint32_t BLOCK_SIZE = 256;
	DCONST uint32_t TOTAL_ELEMENTS = 1024 * 256;
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

	// Descriptors
	AnthemDescriptorPool* descSrc;
	AnthemDescriptorPool* descDst;
	AnthemDescriptorPool* descLocalPrefixSum;
	AnthemDescriptorPool* descGlobalPrefixSum;
	AnthemDescriptorPool* descUniform;

	// Command Bufer
	uint32_t sortCmd = 0;

	// Copyback Buffer
	int* copyback;

	// Active Params
	uint32_t curIteration = 0;

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

	std::optional<std::function<void(Asset::ShaderBuffer*)>> cFunc = std::nullopt;
	st.rd.createShaderStorageBuffer(&st.gSrc, sc.TOTAL_ELEMENTS, 0, st.descSrc, cFunc, -1);
	st.rd.createShaderStorageBuffer(&st.gDst, sc.TOTAL_ELEMENTS, 0, st.descDst, cFunc, -1);
	st.rd.createShaderStorageBuffer(&st.gLocalPrefixSum, sc.TOTAL_ELEMENTS, 0, st.descLocalPrefixSum, cFunc, -1);
	st.rd.createShaderStorageBuffer(&st.gGlobalPrefixSum, sc.TOTAL_ELEMENTS, 0, st.descGlobalPrefixSum, cFunc, -1);

	st.rd.createUniformBuffer(&st.uniform,0, st.descUniform, -1);
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
	st.pLocal->setDescriptorLayouts({
		{st.descSrc,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descGlobalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descLocalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.pLocal->buildComputePipeline();

	st.pGather = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pGather->shaderPath.computeShader = sc.SHADER_GATHER;
	st.pGather->setDescriptorLayouts({
		{st.descGlobalPrefixSum,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descUniform,AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.pGather->buildComputePipeline();

	st.pScatter = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pScatter->shaderPath.computeShader = sc.SHADER_SCATTER;
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

	st.rd.

	st.rd.drEndCommandRecording(st.sortCmd);
}

int main() {
	ANTH_LOGI("Hello, World!");
	return 0;
}