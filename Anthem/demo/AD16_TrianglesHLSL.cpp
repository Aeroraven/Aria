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
	AnthemVertexBufferImpl<
		AtAttributeVecf<3>  //Pos
	>* vx = nullptr;
	AnthemInstancingVertexBufferImpl<
		AtAttributeVecf<4>, //Rot
		AtAttributeVecf<4>, //Translation
		AtAttributeVecf<4>  //Color
	>* vix = nullptr;
	AnthemIndexBuffer* ix = nullptr;

	AnthemDescriptorPool* descUni = nullptr;
	AnthemUniformBufferImpl<
		AtUniformMatf<4>,	//Proj
		AtUniformMatf<4>,	//View
		AtUniformMatf<4>	//Model
	>* uniform = nullptr;

	AnthemShaderFilePaths path;
	AnthemShaderModule* shader = nullptr;
	AnthemRenderPass* pass = nullptr;
	AnthemSwapchainFramebuffer* framebuffer = nullptr;
	AnthemGraphicsPipeline* pipe = nullptr;
	AnthemDepthBuffer* depth = nullptr;

	AnthemRenderPassSetupOption ropt;
	AnthemGraphicsPipelineCreateProps copt;

	AnthemSimpleToyRenderer renderer;
	AnthemConfig config;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);

	const int instNum = 2000;
	const int inFlight = 2;
	float time = 0.0f;
}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "tri\\tri.";
	st += x;
	st += ".hlsl.spv";
	return st;
}

void initialize() {
	st.renderer.setConfig(&st.config);
	st.renderer.initialize();
	int rdH, rdW;
	st.renderer.exGetWindowSize(rdH, rdW);
	st.camera.specifyFrustum((float)AT_PI / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 0, -2);
}


void createVertex() {
	st.renderer.createVertexBuffer(&st.vx);
	st.vx->setTotalVertices(3);
	st.vx->insertData(0, { -0.05,0.0,0.0 });
	st.vx->insertData(1, { 0,0.05,0.0 });
	st.vx->insertData(2, { 0.05,0.0,0.0 });

	st.renderer.createIndexBuffer(&st.ix);
	st.ix->setIndices({ 0,1,2 });
}

void createInstancing() {
	st.renderer.createInstancingBuffer(&st.vix);
	st.vix->setAttrBindingPoint({ 1,2,3 });
	st.vix->setTotalVertices(st.instNum);
	for (int i = 0; i < st.instNum; i++) {
		auto trans = AnthemLinAlg::randomVectorIid3<float>();
		auto rot = AnthemLinAlg::randomVector3<float>();
		auto ang = AnthemLinAlg::randomNumber<float>();
		auto rgba = AnthemLinAlg::randomRgba<float>();

		std::array<float, 4> rotv;
		std::array<float, 4> tranv;
		auto rota = rot.toStdArray();
		auto trana = rot.toStdArray();
		std::copy(rota.begin(), rota.end(), rotv.begin());
		std::copy(trana.begin(), trana.end(), tranv.begin());
		rotv[3] = ang;
		rotv[0] = 0;
		rotv[1] = 1;
		rotv[2] = 0;

		tranv[0] = 3.5;
		tranv[1] = 3.5;
		tranv[2] = 0;
		tranv[3] = 0;

		st.vix->insertData(i, rotv, tranv, rgba.toStdArray());
	}
}

void createUniform() {
	st.renderer.createDescriptorPool(&st.descUni);
	st.renderer.createUniformBuffer(&st.uniform, 0, st.descUni, -1);
}

void updateUniform() {
	AtMatf4 proj, view, model;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	model = AnthemLinAlg::axisAngleRotationTransform3<float>({ 0.0f,1.0f,0.0f }, static_cast<float>(glfwGetTime()));
	
	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	model.columnMajorVectorization(lm);

	st.uniform->specifyUniforms(pm, vm, lm);
	for (int i = 0; i < st.inFlight; i++) {
		st.uniform->updateBuffer(i);
	}
}

void createPipeline() {
	st.path.vertexShader = getShader("vert");
	st.path.fragmentShader = getShader("frag");
	st.renderer.createShader(&st.shader, &st.path);

	st.renderer.createDepthBuffer(&st.depth, false);

	st.ropt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	st.ropt.msaaType = AT_ARPMT_NO_MSAA;
	st.renderer.setupRenderPass(&st.pass, &st.ropt, st.depth);

	st.renderer.createSwapchainImageFramebuffers(&st.framebuffer, st.pass, st.depth);

	AnthemDescriptorSetEntry dseUni{
		.descPool = st.descUni,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	st.copt.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
	st.copt.vertStageLayout = { st.vx,st.vix };
	st.renderer.createGraphicsPipelineCustomized(&st.pipe, { dseUni }, {}, st.pass, st.shader, st.vx, &st.copt);
}

void recordCommand() {
	auto& rd = st.renderer;
	for (int i = 0; i < st.inFlight; i++) {
		rd.drStartCommandRecording(i);
		rd.drStartRenderPass(st.pass, (AnthemFramebuffer*)(st.framebuffer->getFramebufferObject(i)), i, false);
		rd.drSetViewportScissor(i);
		rd.drBindVertexBufferMultiple({ st.vx,st.vix }, i);
		rd.drBindIndexBuffer(st.ix,i);
		rd.drBindGraphicsPipeline(st.pipe, i);

		AnthemDescriptorSetEntry dseUni{
			.descPool = st.descUni,
			.descSetType = AT_ACDS_UNIFORM_BUFFER,
			.inTypeIndex = static_cast<uint32_t>(i)
		};
		rd.drBindDescriptorSetCustomizedGraphics({ dseUni }, st.pipe, i);
		rd.drDrawInstanced(3, st.instNum, i);
		rd.drEndRenderPass(i);
		rd.drEndCommandRecording(i);
	}
}

void mainLoop() {
	static int cur = -1;
	cur++;
	cur %= 2;
	uint32_t imgIdx;
	updateUniform();
	st.renderer.drPrepareFrame(cur, &imgIdx);
	st.renderer.drSubmitBufferPrimaryCall(cur, cur);
	st.renderer.drPresentFrame(cur, cur);
	
}

int main() {
	initialize();
	createUniform();
	createInstancing();
	createVertex();
	createPipeline();
	st.renderer.registerPipelineSubComponents();
	recordCommand();
	st.renderer.setDrawFunction(mainLoop);
	st.renderer.startDrawLoopDemo();
	st.renderer.finalize();
	return 0;
}