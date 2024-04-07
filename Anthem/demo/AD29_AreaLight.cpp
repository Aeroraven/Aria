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
#include "../include/components/passhelper/AnthemPassHelper.h"
#include "../include/components/passhelper/AnthemComputePassHelper.h"
#include "../include/components/passhelper/AnthemSequentialCommand.h"
#include "../include/components/postprocessing/AnthemPostIdentity.h"
#include "../include/components/math/AnthemLowDiscrepancySequence.h"
#include "../include/components/postprocessing/AnthemFXAA.h"

#include "../include/external/LTCLightParameters.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::Components::PassHelper;
using namespace Anthem::Components::Postprocessing;
using namespace Anthem::Components::Math;
using namespace Anthem::External;
using namespace Anthem::Core;

struct Stage {
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camMain = AnthemCamera(AT_ACPT_PERSPECTIVE);

	AnthemVertexBufferImpl<
		AtAttributeVecf<4>, //Position
		AtAttributeVecf<4>, //Normal
		AtAttributeVecf<4>	//Texcoord & Ground
	>* vx;
	AnthemIndexBuffer* ix;

	std::unique_ptr<AnthemPassHelper> mainPass;
	std::unique_ptr<AnthemSequentialCommand> passSeq[2];

	AnthemUniformBufferImpl<
		AnthemUniformMatf<4>,
		AnthemUniformMatf<4>,
		AnthemUniformMatf<4>,
		AnthemUniformVecf<4>
	>* ubuf;
	AnthemDescriptorPool* descUni;

	AnthemUniformBufferImpl<
		AnthemUniformVecfArray<4, 4>
	>* uLbuf;
	AnthemDescriptorPool* descUniL;

	AnthemImage* lut1;
	AnthemImage* lut2;
	AnthemDescriptorPool* descLut;
}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "ltcal\\ltc.";
	st += x;
	st += ".hlsl.spv";
	return st;
}

void initialize() {
	st.cfg.demoName = "29. LTC-based Area Light";
	st.cfg.vkcfgPreferSrgbImagePresentation = false;
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.camMain.specifyFrustum((float)AT_PI * 1.0f / 2.0f, 0.01f, 1000.0f, 1.0f * rdW / rdH);
	st.camMain.specifyPosition(0, 1, -15);
	st.camMain.specifyFrontEyeRay(0, 0, 1);
}

void createLUT() {
	st.rd.createDescriptorPool(&st.descLut);

	AnthemImageCreateProps prop1, prop2;
	prop1.texWidth = 64;
	prop1.texHeight = 64;
	prop1.texChannel = 4;
	prop1.useFloatData = true;
	prop1.format = AT_IF_SIGNED_FLOAT32;
	prop2 = prop1;

	prop1.texDataFloat = AcExternal::AreaLights::LTC1;
	prop2.texDataFloat = AcExternal::AreaLights::LTC2;
	st.rd.createTexture2(&st.lut1, &prop1, nullptr, 0, -1, true);
	st.rd.createTexture2(&st.lut2, &prop2, nullptr, 0, -1, true);
	st.rd.addSamplerArrayToDescriptor({ st.lut1,st.lut2 }, st.descLut, 0, -1);
}

void createGeometry() {
	st.rd.createVertexBuffer(&st.vx);
	st.rd.createIndexBuffer(&st.ix);
	st.vx->setTotalVertices(8);
	st.vx->insertData(0, { -100,0,-100,1 }, { 0,1,0,0 }, { 0,0,0,0 });
	st.vx->insertData(1, { 100,0,-100,1 }, { 0,1,0,0 }, { 1,0,0,0 });
	st.vx->insertData(2, { 100,0,100,1 }, { 0,1,0,0 }, { 1,1,0,0 });
	st.vx->insertData(3, { -100,0,100,1 }, { 0,1,0,0 }, { 0,1,0,0 });
	st.vx->insertData(4, { -5,11,0,1 }, { 0,0,-1,0 }, { 0,0,1,0 });
	st.vx->insertData(5, { 5,11,0,1 }, { 0,0,-1,0 }, { 0,0,1,0 });
	st.vx->insertData(6, { 5,0.1,0,1 }, { 0,0,-1,0 }, { 0,0,1,0 });
	st.vx->insertData(7, { -5,0.1,0,1 }, { 0,0,-1,0 }, { 0,0,1,0 });
	st.ix->setIndices({ 0,1,2,2,3,0,4,5,6,6,7,4 });
}

void createUniform() {
	st.rd.createDescriptorPool(&st.descUni);
	st.rd.createUniformBuffer(&st.ubuf, 0, st.descUni, -1);
	st.rd.createDescriptorPool(&st.descUniL);
	st.rd.createUniformBuffer(&st.uLbuf, 0, st.descUniL, -1);
}

void createMainPass() {
	st.mainPass = std::make_unique<AnthemPassHelper>(&st.rd, static_cast<uint32_t>(st.cfg.vkcfgMaxImagesInFlight));
	st.mainPass->shaderPath.vertexShader = getShader("main.vert");
	st.mainPass->shaderPath.fragmentShader = getShader("main.frag");
	st.mainPass->vxLayout = st.vx;
	st.mainPass->passOpt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	st.mainPass->setDescriptorLayouts({
		{st.descUni,AT_ACDS_UNIFORM_BUFFER,0},
		{st.descLut,AT_ACDS_SAMPLER,0},
		{st.descUniL,AT_ACDS_UNIFORM_BUFFER,0},
	});
	st.mainPass->buildGraphicsPipeline();
}

void recordCommand() {
	st.mainPass->recordCommands([&](uint32_t x) {
		st.rd.drBindVertexBuffer(st.vx, x);
		st.rd.drBindIndexBuffer(st.ix, x);
		st.rd.drDraw(st.ix->getIndexCount(), x);
	});

	st.passSeq[0] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.passSeq[1] = std::make_unique<AnthemSequentialCommand>(&st.rd);
	st.passSeq[0]->setSequence({ { st.mainPass->getCommandIndex(0), ATC_ASCE_GRAPHICS} });
	st.passSeq[1]->setSequence({ { st.mainPass->getCommandIndex(1), ATC_ASCE_GRAPHICS} });
}

void updateUniform() {
	AtMatf4 proj, view, local, pv;
	st.camMain.getProjectionMatrix(proj);
	st.camMain.getViewMatrix(view);
	local = AnthemLinAlg::axisAngleRotationTransform3<float, float>({ 0.0f,1.0f,0.0f }, glfwGetTime() * 0.00f);
	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);
	pv = proj.multiply(view);

	float cPos[4];
	AtVecf3 cPosX;
	st.camMain.getPosition(cPosX);
	for (auto i : AT_RANGE2(3))cPos[i] = cPosX[i];

	float lpos[16] = {
		-5,11,0,1,
		5,11,0,1,
		5,0.1,0,1,
		-5,0.1,0,1
	};
	st.uLbuf->specifyUniforms(lpos);
	st.ubuf->specifyUniforms(pm, vm, lm,cPos);
	for (int i = 0; i < st.cfg.vkcfgMaxImagesInFlight; i++) {
		st.ubuf->updateBuffer(i);
		st.uLbuf->updateBuffer(i);
	}
}

void drawCall() {
	static int cur = 0;
	updateUniform();
	uint32_t imgIdx = 0;
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.passSeq[cur]->executeCommandToStage(imgIdx,  false);
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}

int main() {
	initialize();
	createLUT();
	createGeometry();
	createUniform();
	createMainPass();

	st.rd.registerPipelineSubComponents();
	recordCommand();

	st.rd.setDrawFunction(drawCall);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();

	return 0;
}