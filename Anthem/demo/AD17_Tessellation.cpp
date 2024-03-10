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
	// Core
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);

	// Geometry
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<2>>* vx;
	AnthemIndexBuffer* ix;

	// Descriptor
	AnthemDescriptorPool* desc;
	AnthemUniformBufferImpl<AtUniformMatf<4>, AtUniformMatf<4>, AtUniformMatf<4>>* uniform;

	// Image
	AnthemImage* heightMap;

	// Pipeline
	AnthemRenderPass* pass;
	AnthemGraphicsPipeline* pipe;
	AnthemSwapchainFramebuffer* fb;
	AnthemDepthBuffer* depth;
	AnthemShaderFilePaths path;
	AnthemShaderModule* shader;

	AnthemRenderPassSetupOption ropt;
	AnthemGraphicsPipelineCreateProps copt;

	// Consts
	const int rawGrids = 20;
	const int inFlight = 2;
}st;

void initialize() {
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.camera.specifyFrustum((float)AT_PI / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 1.85, -1.85);
	st.camera.specifyFrontEyeRay(0, -1.85, 1.85);
}

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "tess\\tess.";
	st += x;
	st += ".hlsl.spv";
	return st;
}


void createDesc() {
	st.rd.createDescriptorPool(&st.desc);
}

void loadImage() {
	AnthemImageLoader loader;
	std::string path = ANTH_ASSET_DIR;
	path += "/heightMap.jpg";
	uint32_t height, width, channel;
	uint8_t* data;
	loader.loadImage(path.data(), &width, &height, &channel, &data);

	static AnthemImageCreateProps iprop{
		.texData = data,
		.texWidth = width,
		.texHeight = height,
		.texChannel = channel,
		.mipmap2d = false,
		.msaa = false,
		.format = AT_IF_SRGB_UINT8,
		.usage = AT_IU_TEXTURE,
		.extraAccessStages = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
	};
	st.rd.createTexture2(&st.heightMap, &iprop, st.desc, 0, -1, false);
}

void createUniform() {
	st.rd.createUniformBuffer(&st.uniform, 0, st.desc, -1);
}

void createGeometry() {
	st.rd.createVertexBuffer(&st.vx);
	st.rd.createIndexBuffer(&st.ix);
	st.vx->setTotalVertices(st.rawGrids * st.rawGrids * 4);
	st.vx->setAttrBindingPoint({ 0,1 });
	int cur = 0;
	constexpr static int dirv[4][2] = { {0,0},{1,0},{0,1},{1,1} };
	std::vector<uint32_t> indices;
	auto createVNode = [&](int x, int y) {
		for (int i = 0; i < 4; i++) {
			int ci = x + dirv[i][0];
			int cj = y + dirv[i][1];
			uint32_t locId = cj * st.rawGrids + ci;
			float fx = 1.0f * ci / (st.rawGrids - 1);
			float fy = 1.0f * cj / (st.rawGrids - 1);
			float locX = fx * 1.0 - 0.5;
			float locZ = fy * 1.0 - 0.5;
			indices.push_back(cur);
			st.vx->insertData(cur++, { locX,0.0,locZ,0.0 }, { fx,fy });
		}
	};
	for (int i = 0; i < st.rawGrids; i++) {
		for (int j = 0; j < st.rawGrids; j++) {
			createVNode(i, j);
		}
	}
	st.ix->setIndices(indices);
}

void createPipeline() {
	st.path.tessControlShader = getShader("tesc");
	st.path.tessEvalShader = getShader("tese");
	st.path.vertexShader = getShader("vert");
	st.path.fragmentShader = getShader("frag");
	st.rd.createShader(&st.shader, &st.path);

	st.rd.createDepthBuffer(&st.depth,false);

	st.ropt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	st.ropt.msaaType = AT_ARPMT_NO_MSAA;
	st.rd.setupRenderPass(&st.pass, &st.ropt, st.depth);

	st.rd.createSwapchainImageFramebuffers(&st.fb, st.pass, st.depth);
	
	st.copt.enableTessellation = true;
	st.copt.patchControlPoints = 4;
	st.copt.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_PATCH_LIST;

	AnthemDescriptorSetEntry dseUniform{
		.descPool = st.desc,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};

	AnthemDescriptorSetEntry dseImage{
		.descPool = st.desc,
		.descSetType = AT_ACDS_SAMPLER,
		.inTypeIndex = 0
	};

	st.rd.createGraphicsPipelineCustomized(&st.pipe, { dseUniform,dseImage }, {}, st.pass, st.shader, st.vx, &st.copt);
}

void recordCommand() {
	for (int i = 0; i < st.inFlight; i++) {
		st.rd.drStartCommandRecording(i);
		st.rd.drStartRenderPass(st.pass, st.fb->getFramebufferObjectUnsafe(i), i, false);
		st.rd.drBindGraphicsPipeline(st.pipe, i);
		st.rd.drSetViewportScissor(i);
		st.rd.drSetLineWidth(1, i);
		st.rd.drBindVertexBuffer(st.vx, i);
		st.rd.drBindIndexBuffer(st.ix, i);

		AnthemDescriptorSetEntry dseUniform{
			.descPool = st.desc,
			.descSetType = AT_ACDS_UNIFORM_BUFFER,
			.inTypeIndex = static_cast<uint32_t>(i)
		};
		AnthemDescriptorSetEntry dseImage{
			.descPool = st.desc,
			.descSetType = AT_ACDS_SAMPLER,
			.inTypeIndex = static_cast<uint32_t>(1 - i)
		};

		st.rd.drBindDescriptorSetCustomizedGraphics({ dseUniform,dseImage }, st.pipe, i);
		st.rd.drDrawInstanced(st.ix->getIndexCount(), 1, i);
		st.rd.drEndRenderPass(i);
		st.rd.drEndCommandRecording(i);
	}
}

void updateUniform() {
	AtMatf4 proj, view, model;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	model = AnthemLinAlg::axisAngleRotationTransform3<float>({ 0.0f,1.0f,0.0f }, 0.1f * static_cast<float>(glfwGetTime()));

	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	model.columnMajorVectorization(lm);

	st.uniform->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.inFlight; i++) {
		st.uniform->updateBuffer(i);
	}
}

void mainLoop() {
	updateUniform();
	static int cur = 0;
	uint32_t imgIdx;
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.rd.drSubmitBufferPrimaryCall(cur, cur);
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}

int main() {
	initialize();
	createDesc();
	createUniform();
	loadImage();
	createGeometry();
	createPipeline();
	st.rd.registerPipelineSubComponents();
	
	recordCommand();
	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();
}