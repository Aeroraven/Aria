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
	std::string st(ANTH_SHADER_DIR_HLSL"hzb\\hzb.");
	st += x;
	st += ".hlsl.spv";
	return st;
}

#define DCONST static constexpr const
struct Parameters {
	DCONST uint32_t MAX_INSTANCE = 10000;

	DCONST uint32_t GEN_WORLD_X = 128;
	DCONST uint32_t GEN_WORLD_Y = 128;
	DCONST uint32_t GEN_WORLD_Z = 128;

	DCONST uint32_t GEN_THREAD_X = 8;
	DCONST uint32_t GEN_THREAD_Y = 8;
	DCONST uint32_t GEN_THREAD_Z = 8;

	DCONST uint32_t VIEWPORT_SIZE = 2048;

	std::string SHADER_OCCLUDER_GEN = getShader("occluderGen.comp");
	std::string SHADER_OCCLUDEE_GEN = getShader("occludeeGen.comp");

	std::string SHADER_GENERAL_VS = getShader("general.vert");
	std::string SHADER_GENERAL_FS = getShader("general.frag");
	std::string SHADER_BLIT = getShader("blit.comp");
	std::string SHADER_COPY = getShader("copy.comp");
	std::string SHADER_FILTER = getShader("filter.comp");
	std::string SHADER_DRAWGEN = getShader("drawGen.comp");
}sc;
#undef DCONST

struct Assets {
	// Core
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemFrameRateMeter frm = AnthemFrameRateMeter(10);
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);

	// Camera
	AnthemUniformBufferImpl<AtUniformMatf<4>>* uniform;
	AnthemDescriptorPool* descUniform;

	// Geometry
	using Geometry = AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>>*;
	using Instance = AnthemShaderStorageBufferImpl<AtBufVecd4f<1>>*;
	using IndirectEntry = AnthemShaderStorageBufferImpl<AtBufVecd4i<1>,AtBufVecdInt<1>>*;

	Geometry occluder = nullptr;
	Instance occluderPos = nullptr;
	Geometry occludee = nullptr;
	Instance occludeePos = nullptr;
	Instance occludeePosCulled = nullptr;
	IndirectEntry indirectBuffer = nullptr;

	AnthemIndexBuffer* occluderIdx = nullptr;
	AnthemIndexBuffer* occludeeIdx = nullptr;
	AnthemDescriptorPool* descPoolOccluder = nullptr;
	AnthemDescriptorPool* descPoolOccludee = nullptr;
	AnthemDescriptorPool* descPoolOccludeeCulled = nullptr;
	AnthemDescriptorPool* descPoolIndirect = nullptr;
	
	AnthemPushConstantImpl<AtBufVecInt<1>>* pc = nullptr;

	uint32_t occluderNums = 0;
	uint32_t occludeeNums = 0;

	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>>* blitGeom = nullptr;
	AnthemIndexBuffer* blitIdx = nullptr;

	// Passes
	std::unique_ptr<AnthemComputePassHelper> pOccluderGen;
	std::unique_ptr<AnthemComputePassHelper> pOccludeeGen;
	std::unique_ptr<AnthemPassHelper> pOccluderRender;
	std::unique_ptr<AnthemPassHelper> pOccludeeRender;

	std::unique_ptr<AnthemComputePassHelper> pOccludeeFilter;
	std::unique_ptr<AnthemComputePassHelper> pIndirectGen;
	std::unique_ptr<AnthemComputePassHelper> pDepthCopy;
	std::unique_ptr<AnthemComputePassHelper> pDepthBlit;

	std::unique_ptr<AnthemPostIdentity> pPostFx;

	std::unique_ptr<AnthemSequentialCommand> execSeq[2];

	// Render Targets (Although can be done via mipmaps)
	AnthemImage** depthMips;
	AnthemDescriptorPool** descDepth;
	AnthemDescriptorPool* descDepthAll;

	AnthemImage* colorImg;
	AnthemDescriptorPool* descColor;

	// Commands
	uint32_t depthBlitCmd[2];
	uint32_t indirectGenCmd[2];
	AnthemViewport* viewport;
}st;

void createViewport() {
	st.rd.createViewportCustom(&st.viewport, sc.VIEWPORT_SIZE, sc.VIEWPORT_SIZE, 0, 1);
	st.rd.createDescriptorPool(&st.descColor);
	st.rd.createColorAttachmentImage(&st.colorImg, st.descColor, 0, AT_IF_SIGNED_FLOAT32, false, -1, false, sc.VIEWPORT_SIZE, sc.VIEWPORT_SIZE);
}

void createDepthMipMaps() {
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);

	int maxval = sc.VIEWPORT_SIZE;
	int mipLevels = 1, simw = maxval;
	while (simw != 1) {
		if (simw % 2 == 1)simw++;
		simw /= 2;
		mipLevels++;
	}

	st.depthMips = new AnthemImage * [mipLevels];
	st.descDepth = new AnthemDescriptorPool * [mipLevels];

	std::vector<AnthemImageContainer*> cts;
	int sW = sc.VIEWPORT_SIZE, sH = sc.VIEWPORT_SIZE;
	for (auto i : AT_RANGE2(mipLevels)) {
		st.rd.createDescriptorPool(&st.descDepth[i]);
		st.rd.createTexture(&st.depthMips[i], nullptr, 0, sW, sH, 1, 0, false, false, AT_IF_SIGNED_FLOAT32_MONO, -1, false, AT_IU_COMPUTE_OUTPUT);
		st.depthMips[i]->toGeneralLayout();
		if(sW%2==1)sW++;
		if(sH%2==1)sH++;
		sW /= 2;
		sH /= 2;
		st.rd.addStorageImageArrayToDescriptor({ st.depthMips[i] }, st.descDepth[i], 0, -1);
		cts.push_back(st.depthMips[i]);
	}
	st.rd.createDescriptorPool(&st.descDepthAll);
	st.rd.addStorageImageArrayToDescriptor(cts, st.descDepthAll, 0, -1);
}

void createBlitGeometry() {
	st.rd.createVertexBuffer(&st.blitGeom);
	st.rd.createIndexBuffer(&st.blitIdx);

	st.blitGeom->setTotalVertices(4);
	st.blitGeom->insertData(0, { -1,-1,0,1 }, { 0,0,0,0 });
	st.blitGeom->insertData(1, { 1,-1,0,1 }, { 1,0,0,0 });
	st.blitGeom->insertData(2, { 1,1,0,1 }, { 1,1,0,0 });
	st.blitGeom->insertData(3, { -1,1,0,1 }, { 0,1,0,0 });

	std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
	st.blitIdx->setIndices(indices);
}



void createGeometry() {
	st.rd.createVertexBuffer(&st.occluder);
	st.rd.createVertexBuffer(&st.occludee);

	// Occluder
	st.occluder->setTotalVertices(8);
	st.occluder->insertData(0, { -1,-1,-1,1 }, { 1,0,0,0 });
	st.occluder->insertData(1, { 1,-1,-1,1 }, { 1,0,0,0 });
	st.occluder->insertData(2, { 1,1,-1,1 }, { 1,0,0,0 });
	st.occluder->insertData(3, { -1,1,-1,1 }, { 1,0,0,0 });
	st.occluder->insertData(4, { -1,-1,1,1 }, { 1,0,0,0 });
	st.occluder->insertData(5, { 1,-1,1,1 }, { 1,0,0,0 });
	st.occluder->insertData(6, { 1,1,1,1 }, { 1,0,0,0 });
	st.occluder->insertData(7, { -1,1,1,1 }, { 1,0,0,0 });

	std::vector<uint32_t> indices = {
		0,1,2, 2,3,0,
		4,5,6, 6,7,4,
		0,4,7, 7,3,0,
		1,5,6, 6,2,1,
		0,1,5, 5,4,0,
		3,2,6, 6,7,3
	};
	st.rd.createIndexBuffer(&st.occluderIdx);
	st.occluderIdx->setIndices(indices);

	// Occludee Sphere
	std::vector < std::array<float, 4>> spherePts;
	std::vector<uint32_t> sphereIdx;
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			float x = cos(2 * 3.14159265359f / 99 * i) * sin(3.14159265359f / 99 * j);
			float y = sin(2 * 3.14159265359f / 99 * i) * sin(3.14159265359f / 99 * j);
			float z = cos(3.14159265359f / 99 * j);
			spherePts.push_back({ x,y,z,1 });
		}
	}
	for (int i = 0; i < 99; i++) {
		for (int j = 0; j < 99; j++) {
			sphereIdx.push_back(i * 100 + j);
			sphereIdx.push_back((i + 1) * 100 + j);
			sphereIdx.push_back(i * 100 + j + 1);
			sphereIdx.push_back((i + 1) * 100 + j);
			sphereIdx.push_back((i + 1) * 100 + j + 1);
			sphereIdx.push_back(i * 100 + j + 1);
		}
	}
	st.rd.createIndexBuffer(&st.occludeeIdx);
	st.occludeeIdx->setIndices(sphereIdx);

	st.occludee->setTotalVertices(spherePts.size());
	for (int i = 0; i < spherePts.size(); i++) {
		st.occludee->insertData(i, spherePts[i], { 0,0,1,0 });
	}

	// Instancing Buffer
	st.rd.createDescriptorPool(&st.descPoolOccluder);
	st.rd.createDescriptorPool(&st.descPoolOccludee);
	st.rd.createDescriptorPool(&st.descPoolOccludeeCulled);
	st.rd.createDescriptorPool(&st.descPoolIndirect);
	using ssboType  = decltype(st.occluderPos);
	std::optional< std::function<void(ssboType)>> w = std::nullopt;
	using ssboType2 = decltype(st.indirectBuffer);
	std::optional< std::function<void(ssboType2)>> w2 = std::nullopt;

	st.rd.createShaderStorageBuffer(&st.occluderPos, sc.MAX_INSTANCE, 0, st.descPoolOccluder,w , 0);
	st.rd.createShaderStorageBuffer(&st.occludeePos, sc.MAX_INSTANCE, 0, st.descPoolOccludee, w, 0);
	st.rd.createShaderStorageBuffer(&st.occludeePosCulled, sc.MAX_INSTANCE, 0, st.descPoolOccludeeCulled, w, 0);
	st.rd.createShaderStorageBuffer(&st.indirectBuffer, sc.MAX_INSTANCE, 0, st.descPoolIndirect, w2, 0);

	st.occluderPos->useAsInstancingBuffer();
	st.occludeePos->useAsInstancingBuffer();
	st.occludeePosCulled->useAsInstancingBuffer();
	st.occluderPos->setAttrBindingPoint({ 2 });
	st.occludeePos->setAttrBindingPoint({ 2 });
	st.occludeePosCulled->setAttrBindingPoint({ 2 });
}

void createComputePass() {
	st.rd.createPushConstant(&st.pc);
	st.pc->enableShaderStage(AT_APCS_COMPUTE);

	#define CREATE_COMPUTE_PASS(x) x = std::make_unique<AnthemComputePassHelper>(&st.rd,2);
	CREATE_COMPUTE_PASS(st.pOccludeeGen);
	st.pOccludeeGen->shaderPath.computeShader = sc.SHADER_OCCLUDEE_GEN;
	st.pOccludeeGen->workGroupSize[0] = sc.GEN_WORLD_X / sc.GEN_THREAD_X;
	st.pOccludeeGen->workGroupSize[1] = sc.GEN_WORLD_Y / sc.GEN_THREAD_Y;
	st.pOccludeeGen->workGroupSize[2] = sc.GEN_WORLD_Z / sc.GEN_THREAD_Z;
	st.pOccludeeGen->setDescriptorLayouts({
		{st.descPoolOccludee,AT_ACDS_SHADER_STORAGE_BUFFER,0}
	});
	st.pOccludeeGen->buildComputePipeline();

	CREATE_COMPUTE_PASS(st.pOccluderGen);
	st.pOccluderGen->shaderPath.computeShader = sc.SHADER_OCCLUDER_GEN;
	st.pOccluderGen->workGroupSize[0] = sc.GEN_WORLD_X / sc.GEN_THREAD_X;
	st.pOccluderGen->workGroupSize[1] = sc.GEN_WORLD_Y / sc.GEN_THREAD_Y;
	st.pOccluderGen->workGroupSize[2] = sc.GEN_WORLD_Z / sc.GEN_THREAD_Z;
	st.pOccluderGen->setDescriptorLayouts({
		{st.descPoolOccluder,AT_ACDS_SHADER_STORAGE_BUFFER,0}
	});
	st.pOccluderGen->buildComputePipeline();

	CREATE_COMPUTE_PASS(st.pDepthBlit);
	st.pDepthBlit->shaderPath.computeShader = sc.SHADER_BLIT;
	st.pDepthBlit->workGroupSize[0] = 1;
	st.pDepthBlit->workGroupSize[1] = 1;
	st.pDepthBlit->workGroupSize[2] = 1;
	st.pDepthBlit->setDescriptorLayouts({
		{st.descDepth[0],AT_ACDS_STORAGE_IMAGE,0},
		{st.descDepth[1],AT_ACDS_STORAGE_IMAGE,0}
	});
	st.pDepthBlit->buildComputePipeline();

	CREATE_COMPUTE_PASS(st.pOccludeeFilter);
	st.pOccludeeFilter->shaderPath.computeShader = sc.SHADER_FILTER;
	st.pOccludeeFilter->workGroupSize[0] = 1;
	st.pOccludeeFilter->workGroupSize[1] = 1;
	st.pOccludeeFilter->workGroupSize[2] = 1;
	st.pOccludeeFilter->setDescriptorLayouts({
		{st.descUniform , AT_ACDS_UNIFORM_BUFFER,0},
		{st.descDepthAll,AT_ACDS_STORAGE_IMAGE,0},
		{st.descPoolOccludeeCulled,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descPoolOccludee,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		});
	st.pOccludeeFilter->buildComputePipeline();

	CREATE_COMPUTE_PASS(st.pIndirectGen);
	st.pIndirectGen->shaderPath.computeShader = sc.SHADER_DRAWGEN;
	st.pIndirectGen->workGroupSize[0] = 1;
	st.pIndirectGen->workGroupSize[1] = 1;
	st.pIndirectGen->workGroupSize[2] = 1;
	st.pIndirectGen->pushConstants = { st.pc };
	st.pIndirectGen->setDescriptorLayouts({
		{st.descPoolOccludeeCulled,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		{st.descPoolIndirect,AT_ACDS_SHADER_STORAGE_BUFFER,0},
	});
	st.pIndirectGen->buildComputePipeline();

	#undef CREATE_COMPUTE_PASS
}

void createGraphicsPass() {
	st.pOccluderRender = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.pOccluderRender->shaderPath.vertexShader = sc.SHADER_GENERAL_VS;
	st.pOccluderRender->shaderPath.fragmentShader = sc.SHADER_GENERAL_FS;
	st.pOccluderRender->pipeOpt.vertStageLayout = { st.occluder,st.occluderPos };
	st.pOccluderRender->setRenderTargets({ st.colorImg });
	st.pOccluderRender->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.pOccluderRender->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.pOccluderRender->viewport = st.viewport;
	st.pOccluderRender->setDescriptorLayouts({
		{st.descUniform , AT_ACDS_UNIFORM_BUFFER,0}
		});
	st.pOccluderRender->enableDepthSampler = true;
	st.pOccluderRender->buildGraphicsPipeline();


	st.pOccludeeRender = std::make_unique<AnthemPassHelper>(&st.rd, 2);
	st.pOccludeeRender->shaderPath.vertexShader = sc.SHADER_GENERAL_VS;
	st.pOccludeeRender->shaderPath.fragmentShader = sc.SHADER_GENERAL_FS;
	st.pOccludeeRender->pipeOpt.vertStageLayout = { st.occludee,st.occludeePos };
	st.pOccludeeRender->viewport = st.viewport;
	st.pOccludeeRender->passOpt.clearColorAttachmentOnLoad = { false };
	st.pOccludeeRender->passOpt.clearDepthAttachmentOnLoad = false;
	st.pOccludeeRender->passOpt.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.pOccludeeRender->passOpt.colorAttachmentFormats = { AT_IF_SIGNED_FLOAT32 };
	st.pOccludeeRender->viewport = st.viewport;
	st.pOccludeeRender->setRenderTargets({ st.colorImg });
	st.pOccludeeRender->setDepthFromPass(*st.pOccluderRender);
	st.pOccludeeRender->setDescriptorLayouts({
		{st.descUniform , AT_ACDS_UNIFORM_BUFFER,0}
	});
	st.pOccludeeRender->buildGraphicsPipeline();

	st.pPostFx = std::make_unique<AnthemPostIdentity>(&st.rd, 2);
	st.pPostFx->addInput({
		{st.descColor,AT_ACDS_SAMPLER,0}
	});
	st.pPostFx->prepare(false);
}

void createDepthCopyPass() {
	st.pDepthCopy = std::make_unique<AnthemComputePassHelper>(&st.rd, 2);
	st.pDepthCopy->shaderPath.computeShader = sc.SHADER_COPY;
	st.pDepthCopy->workGroupSize[0] = 1;
	st.pDepthCopy->workGroupSize[1] = 1;
	st.pDepthCopy->workGroupSize[2] = 1;
	st.pDepthCopy->setDescriptorLayouts({
		{st.pOccluderRender->getDepthDescriptor(0),AT_ACDS_SAMPLER,0},
		{st.descDepth[0],AT_ACDS_STORAGE_IMAGE,0}
	});
	st.pDepthCopy->buildComputePipeline();
}

void recordIndirectDrawGeneration() {
	for (auto i : AT_RANGE2(2)) {
		st.rd.drAllocateCommandBuffer(&st.indirectGenCmd[i]);
		auto c = st.indirectGenCmd[i];
		st.rd.drStartCommandRecording(c);

		// Dispatch Culling
		st.rd.drBindComputePipeline(st.pOccludeeFilter->pipeline, c);
		st.rd.drBindDescriptorSetCustomizedCompute({
			{st.descUniform , AT_ACDS_UNIFORM_BUFFER,0},
			{st.descDepthAll,AT_ACDS_STORAGE_IMAGE,0},
			{st.descPoolOccludeeCulled,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descPoolOccludee,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			}, st.pOccludeeFilter->pipeline, c);
		st.rd.drComputeDispatch(c, std::min(st.occludeeNums, sc.MAX_INSTANCE), 1, 1);

		// Generate Indirect Buffer
		st.rd.drBindComputePipeline(st.pIndirectGen->pipeline, c);
		st.rd.drPushConstantsCompute(st.pc, st.pIndirectGen->pipeline, c);
		st.rd.drBindDescriptorSetCustomizedCompute({
			{st.descPoolOccludeeCulled,AT_ACDS_SHADER_STORAGE_BUFFER,0},
			{st.descPoolIndirect,AT_ACDS_SHADER_STORAGE_BUFFER,0},
		}, st.pIndirectGen->pipeline, c);
		st.rd.drComputeDispatch(c, 1, 1, 1);


		st.rd.drEndCommandRecording(c);
	}
		
}

void recordDepthBlit() {
	for (auto k : AT_RANGE2(2)) {
		st.rd.drAllocateCommandBuffer(&st.depthBlitCmd[k]);
		auto c = st.depthBlitCmd[k];
		int rdH, rdW;
		st.rd.exGetWindowSize(rdH, rdW);
		st.rd.drStartCommandRecording(c);

		// Change Layout
		st.rd.drSetDepthImageLayoutSimple(st.pOccluderRender->getDepthBuffer(0),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,c);

		// Copy Depth
		st.rd.drBindComputePipeline(st.pDepthCopy->pipeline, c);
		st.rd.drBindDescriptorSetCustomizedCompute({
			{st.pOccluderRender->getDepthDescriptor(0),AT_ACDS_SAMPLER,0},
			{st.descDepth[0],AT_ACDS_STORAGE_IMAGE,0}
			}, st.pDepthCopy->pipeline, c);
		st.rd.drComputeDispatch(c, sc.VIEWPORT_SIZE, sc.VIEWPORT_SIZE, 1);

		// Blit Depth
		int sH = sc.VIEWPORT_SIZE, sW = sc.VIEWPORT_SIZE;
		int base = 0;
		while (std::max(sH, sW) != 1) {
			st.rd.drBindComputePipeline(st.pDepthBlit->pipeline, c);
			st.rd.drBindDescriptorSetCustomizedCompute({
				{st.descDepth[base],AT_ACDS_STORAGE_IMAGE,0},
				{st.descDepth[base + 1],AT_ACDS_STORAGE_IMAGE,0}
				}, st.pDepthBlit->pipeline, c);

			if (sH % 2 == 1)sH++;
			if (sW % 2 == 1)sW++;
			sH /= 2;
			sW /= 2;
			st.rd.drComputeDispatch(c, sW, sH, 1);
			base++;
		}

		// Recover Layout
		st.rd.drSetDepthImageLayoutSimple(st.pOccluderRender->getDepthBuffer(0),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, c);
		st.rd.drEndCommandRecording(c);
	}
}

void instanceGeneration() {
	st.occludeePos->setAtomicCounter(0, 0);
	st.occludeePosCulled->setAtomicCounter(0, 0);

	uint32_t cmdIdx;
	AnthemFence* fence;
	st.rd.drAllocateCommandBuffer(&cmdIdx);
	st.rd.createFence(&fence);
	st.rd.drStartCommandRecording(cmdIdx);

	st.rd.drBindComputePipeline(st.pOccludeeGen->pipeline, cmdIdx);
	st.rd.drBindDescriptorSetCustomizedCompute({ {st.descPoolOccludee,AT_ACDS_SHADER_STORAGE_BUFFER,0} }, st.pOccludeeGen->pipeline, cmdIdx);
	st.rd.drComputeDispatch(cmdIdx, sc.GEN_WORLD_X / sc.GEN_THREAD_X, sc.GEN_THREAD_Y / sc.GEN_THREAD_Y, sc.GEN_WORLD_Z / sc.GEN_THREAD_Z);

	st.rd.drBindComputePipeline(st.pOccluderGen->pipeline, cmdIdx);
	st.rd.drBindDescriptorSetCustomizedCompute({ {st.descPoolOccluder,AT_ACDS_SHADER_STORAGE_BUFFER,0} }, st.pOccluderGen->pipeline, cmdIdx);
	st.rd.drComputeDispatch(cmdIdx, sc.GEN_WORLD_X / sc.GEN_THREAD_X, sc.GEN_THREAD_Y / sc.GEN_THREAD_Y, sc.GEN_WORLD_Z / sc.GEN_THREAD_Z);

	st.rd.drEndCommandRecording(cmdIdx);
	fence->resetFence();
	st.rd.drSubmitCommandBufferCompQueueGeneralA(cmdIdx, {}, {}, fence);
	fence->waitAndReset();

	st.occludeeNums = st.occludeePos->getAtomicCounter(0);
	st.occluderNums = st.occluderPos->getAtomicCounter(0);
	int x = st.occludeeNums;
	st.pc->setConstant(&x);
	ANTH_LOGI("Instances Generated");
}

void recordGraphicsCommands() {
	st.pOccludeeRender->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBufferEx2(st.occludee, st.occludeePos, 0, x);
		st.rd.drBindIndexBuffer(st.occludeeIdx,x);
		st.rd.drDrawInstanced(st.occludeeIdx->getIndexCount(), std::min(sc.MAX_INSTANCE, st.occludeeNums), x);
	});
	st.pOccluderRender->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBufferEx2(st.occluder, st.occluderPos, 0, x);
		st.rd.drBindIndexBuffer(st.occluderIdx, x);
		st.rd.drDrawInstanced(st.occluderIdx->getIndexCount(), std::min(sc.MAX_INSTANCE, st.occluderNums), x);
	});
	st.pPostFx->recordCommand();

	for (auto i : AT_RANGE2(2)) {
		st.execSeq[i] = std::make_unique<AnthemSequentialCommand>(&st.rd);
		st.execSeq[i]->setSequence({
			{st.pOccluderRender->getCommandIndex(i),ATC_ASCE_GRAPHICS},
			{st.depthBlitCmd[i],ATC_ASCE_COMPUTE},
			{st.indirectGenCmd[i],ATC_ASCE_COMPUTE},
			{st.pOccludeeRender->getCommandIndex(i),ATC_ASCE_GRAPHICS},
			{st.pPostFx->getCommandIdx(i),ATC_ASCE_GRAPHICS}
		});
	}
}

void createUniform() {
	st.rd.createDescriptorPool(&st.descUniform);
	st.rd.createUniformBuffer(&st.uniform, 0, st.descUniform, -1);
}

void updateUniform() {
	AtMatf4 proj, view, all;
	AtVecf3 camPos;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	all = proj.multiply(view);

	float pm[16];
	all.columnMajorVectorization(pm);
	st.uniform->specifyUniforms(pm);
	for (auto i : AT_RANGE2(2)) {
		st.uniform->updateBuffer(i);
	}
}

void mainLoop() {
	static int cur = 0;
	uint32_t imgIdx;
	updateUniform();
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.execSeq[cur]->executeCommandToStage(imgIdx, false, false, st.pOccludeeRender->getSwapchainBuffer());
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}

void initialize() {
	st.cfg.demoName = "34. Hierarchical Z-Buffer / Occlusion Culling";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.camera.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.01f, 1000.0f, 1.0f * rdW/rdH);
	st.camera.specifyPosition(0, 0, -25);
	st.camera.specifyFrontEyeRay(0, 0, 1);
}

int main() {
	initialize();
	createViewport();
	createUniform();
	createDepthMipMaps();
	createGeometry();
	createComputePass();
	createGraphicsPass();
	createDepthCopyPass();

	st.rd.registerPipelineSubComponents();
	instanceGeneration();
	recordDepthBlit();
	recordIndirectDrawGeneration();
	recordGraphicsCommands();

	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();
	return 0;
}