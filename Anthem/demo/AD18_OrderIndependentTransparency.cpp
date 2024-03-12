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

using namespace Anthem::Components::Camera;
using namespace Anthem::External;
using namespace Anthem::Core;

struct Stage {
	// Main
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);

	// Geometry
	AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>>* vx;
	AnthemIndexBuffer* ixTransparent;
	AnthemIndexBuffer* ixOpaque;

	// Desc
	AnthemDescriptorPool* descAccuAtt;
	AnthemDescriptorPool* descRevealAtt;
	AnthemDescriptorPool* descSolidAtt;
	AnthemPushConstantImpl<
		AtBufMat4f<1>,
		AtBufMat4f<1>,
		AtBufMat4f<1>
	>* pconst;

	// Stage
	AnthemFramebuffer* fbTransparent;
	AnthemFramebuffer* fbOpaque;
	AnthemDepthBuffer* depthTransparent;
	AnthemDepthBuffer* depthOpaque;
	AnthemImage* cbAccu;
	AnthemImage* cbReveal;
	AnthemImage* cbSolid;
	AnthemShaderFilePaths spOpaque;
	AnthemShaderFilePaths spTransparent;
	AnthemShaderModule* shOpaque;
	AnthemShaderModule* shTransparent;
	
	AnthemRenderPass* rpOpaque;
	AnthemRenderPass* rpTransparent;
	AnthemGraphicsPipeline* ppOpaque;
	AnthemGraphicsPipeline* ppTransparent;

	AnthemRenderPassSetupOption roptOpaque;
	AnthemRenderPassSetupOption roptTransparent;
	AnthemGraphicsPipelineCreateProps coptOpaque;
	AnthemGraphicsPipelineCreateProps coptTransparent;

	// Composition
	AnthemVertexBufferImpl<AtAttributeVecf<4>>* vxc;
	AnthemIndexBuffer* ixc;
	AnthemSwapchainFramebuffer* fb;
	AnthemDepthBuffer* depth;
	AnthemShaderFilePaths sp;
	AnthemShaderModule* shader;
	AnthemRenderPass* pass;
	AnthemGraphicsPipeline* pipe;

	AnthemRenderPassSetupOption ropt;
	AnthemGraphicsPipelineCreateProps copt;

	// Const
	const int inFlight = 2;
}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "oit\\oit.";
	st += x;
	st += ".hlsl.spv";
	return st;
}


void initialize() {
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.camera.specifyFrustum((float)AT_PI / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 0, -1.85);
	st.camera.specifyFrontEyeRay(0, 0, 1.85);
}

void createGeometry() {
	st.rd.createVertexBuffer(&st.vx);
	st.vx->setTotalVertices(12);
	float devX[3] = { -1.0,0.0,1.0 };
	float devZ[3] = { 0.0,-1.0,-2.0 };
	float color[3][4] = {
		{0.0,0.0,1.0,0.5},
		{1.0,0.0,0.0,1.0},
		{0.0,1.0,0.0,0.5},
	};
	for (int i = 0; i < 3; i++) {
		st.vx->insertData(i * 4 + 0, { -1.0f + devX[i],1.0f,devZ[i],1.0 }, { color[i][0],color[i][1],color[i][2],color[i][3] });
		st.vx->insertData(i * 4 + 1, {  1.0f + devX[i],1.0f,devZ[i],1.0 }, { color[i][0],color[i][1],color[i][2],color[i][3] });
		st.vx->insertData(i * 4 + 2, { 1.0f + devX[i],-1.0f,devZ[i],1.0 }, { color[i][0],color[i][1],color[i][2],color[i][3] });
		st.vx->insertData(i * 4 + 3, { -1.0f + devX[i],-1.0f,devZ[i],1.0 }, { color[i][0],color[i][1],color[i][2],color[i][3] });
	}
	st.rd.createIndexBuffer(&st.ixOpaque);
	st.ixOpaque->setIndices({ 4,5,6,6,7,4 });
	st.rd.createIndexBuffer(&st.ixTransparent);
	st.ixTransparent->setIndices({ 0,1,2,2,3,0,8,9,10,10,11,8 });
}

void createDescriptor() {
	st.rd.createDescriptorPool(&st.descAccuAtt);
	st.rd.createDescriptorPool(&st.descRevealAtt);
	st.rd.createDescriptorPool(&st.descSolidAtt);
}

void updatePushConstant() {
	st.rd.createPushConstant(&st.pconst);
	st.camera.specifyPosition(0.0f, 0.0f, -1.0f);

	AtMatf4 proj, view, local;
	st.camera.getProjectionMatrix(proj);
	st.camera.getViewMatrix(view);
	local = AnthemLinAlg::identity<float, 4>(); //AnthemLinAlg::axisAngleRotationTransform3<float>({ 0.0f,1.0f,0.0f }, AT_PI / 6.0);

	float pm[16], vm[16], lm[16];
	proj.columnMajorVectorization(pm);
	view.columnMajorVectorization(vm);
	local.columnMajorVectorization(lm);

	st.pconst->setConstant(pm, vm, lm);
	st.pconst->enableShaderStage(AT_APCS_VERTEX);
	st.pconst->enableShaderStage(AT_APCS_FRAGMENT);
}

void createTransparentPipeline() {
	st.spTransparent.vertexShader = getShader("transparent.vert");
	st.spTransparent.fragmentShader = getShader("transparent.frag");
	st.rd.createShader(&st.shTransparent, &st.spTransparent);
	st.rd.createColorAttachmentImage(&st.cbAccu, st.descAccuAtt, 0, AT_IF_SRGB_FLOAT32, false, -1);
	st.rd.createColorAttachmentImage(&st.cbReveal, st.descRevealAtt, 0, AT_IF_SRGB_FLOAT32, false, -1);
	st.rd.createDepthBuffer(&st.depthTransparent,false);

	st.roptTransparent.colorAttachmentFormats = { AT_IF_SRGB_FLOAT32 ,AT_IF_SRGB_FLOAT32 };
	st.roptTransparent.clearColors = { {0.0f,0.0f,0.0f,0.0f},{1.0f,1.0f,1.0f,1.0f} };
	st.roptTransparent.clearColorAttachmentOnLoad = { true,true };
	st.roptTransparent.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.roptTransparent.msaaType = AT_ARPMT_NO_MSAA;
	st.rd.setupRenderPass(&st.rpTransparent, &st.roptTransparent, st.depthTransparent);

	std::vector<const AnthemImage*> colorAtts = { st.cbAccu,st.cbReveal };
	st.rd.createSimpleFramebuffer(&st.fbTransparent, &colorAtts, st.rpTransparent, st.depthTransparent);

	st.coptTransparent.blendPreset = { AnthemBlendPreset::AT_ABP_WEIGHTED_BLENDED_ACCUM, AnthemBlendPreset::AT_ABP_WEIGHTED_BLENDED_REVEAL };
	st.coptTransparent.writeDepthStencil = false;
	st.coptTransparent.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
	st.rd.createGraphicsPipelineCustomized(&st.ppTransparent, {}, { st.pconst }, st.rpTransparent, st.shTransparent, st.vx, &st.coptTransparent);
}

void createOpaquePipeline() {
	st.spOpaque.vertexShader = getShader("opaque.vert");
	st.spOpaque.fragmentShader = getShader("opaque.frag");
	st.rd.createShader(&st.shOpaque, &st.spOpaque);
	st.rd.createDepthBuffer(&st.depthOpaque, false);
	st.rd.createColorAttachmentImage(&st.cbSolid, st.descSolidAtt, 0, AT_IF_SRGB_FLOAT32, false, -1);

	st.roptOpaque.colorAttachmentFormats = { AT_IF_SRGB_FLOAT32 };
	st.roptOpaque.clearColors = { {0,0,0,1} };
	st.roptOpaque.renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS;
	st.roptOpaque.clearColorAttachmentOnLoad = { true };
	st.roptOpaque.msaaType = AT_ARPMT_NO_MSAA;
	st.rd.setupRenderPass(&st.rpOpaque, &st.roptOpaque, st.depthOpaque);

	std::vector<const AnthemImage*> colorAtts = { st.cbSolid };
	st.rd.createSimpleFramebuffer(&st.fbOpaque, &colorAtts, st.rpOpaque, st.depthOpaque);
	st.coptOpaque.blendPreset = { AnthemBlendPreset::AT_ABP_NO_BLEND };
	st.coptOpaque.writeDepthStencil = true;
	st.coptOpaque.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
	st.rd.createGraphicsPipelineCustomized(&st.ppOpaque, {}, { st.pconst }, st.rpOpaque, st.shOpaque, st.vx, &st.coptOpaque);
}

void createCompositionPipeline() {
	st.rd.createVertexBuffer(&st.vxc);
	st.vxc->setTotalVertices(4);
	st.vxc->insertData(0, { -1.0,-1.0,0.0,1.0 });
	st.vxc->insertData(1, { -1.0, 1.0,0.0,1.0 });
	st.vxc->insertData(2, {  1.0, 1.0,0.0,1.0 });
	st.vxc->insertData(3, {  1.0,-1.0,0.0,1.0 });

	st.rd.createIndexBuffer(&st.ixc);
	st.ixc->setIndices({ 0,1,2,2,3,0 });

	st.sp.vertexShader = getShader("composite.vert");
	st.sp.fragmentShader = getShader("composite.frag");
	st.rd.createShader(&st.shader, &st.sp);
	st.rd.createDepthBuffer(&st.depth,false);
	st.ropt.msaaType = AT_ARPMT_NO_MSAA;
	st.ropt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	st.rd.setupRenderPass(&st.pass, &st.ropt, st.depth);

	st.rd.createSwapchainImageFramebuffers(&st.fb, st.pass, st.depth);
	st.copt.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;

	AnthemDescriptorSetEntry dseAccu{ st.descAccuAtt,AT_ACDS_SAMPLER,0 };
	AnthemDescriptorSetEntry dseReveal{ st.descRevealAtt,AT_ACDS_SAMPLER,0 };
	AnthemDescriptorSetEntry dseSolid{ st.descSolidAtt,AT_ACDS_SAMPLER,0 };
	st.rd.createGraphicsPipelineCustomized(&st.pipe, { dseAccu ,dseReveal,dseSolid}, { }, st.pass, st.shader, st.vxc, &st.copt);
}

void recordCommands() {
	auto& r = st.rd;
	for (int i = 0; i < st.inFlight; i++) {
		r.drStartCommandRecording(i);

		r.drStartRenderPass(st.rpTransparent, st.fbTransparent, i, false);
		r.drBindGraphicsPipeline(st.ppTransparent, i);
		r.drSetViewportScissor(i);
		r.drBindVertexBuffer(st.vx, i);
		r.drBindIndexBuffer(st.ixTransparent, i);
		r.drPushConstants(st.pconst, st.ppTransparent, i);
		r.drDraw(st.ixTransparent->getIndexCount(), i);
		r.drEndRenderPass(i);

		r.drStartRenderPass(st.rpOpaque, st.fbOpaque, i, false);
		r.drBindGraphicsPipeline(st.ppOpaque, i);
		r.drSetViewportScissor(i);
		r.drBindVertexBuffer(st.vx, i);
		r.drBindIndexBuffer(st.ixOpaque, i);
		r.drPushConstants(st.pconst, st.ppOpaque, i);
		r.drDraw(st.ixOpaque->getIndexCount(), i);
		r.drEndRenderPass(i);


		r.drStartRenderPass(st.pass, st.fb->getFramebufferObjectUnsafe(i), i, false);
		r.drBindGraphicsPipeline(st.pipe, i);
		r.drSetViewportScissor(i);
		r.drBindVertexBuffer(st.vxc, i);
		r.drBindIndexBuffer(st.ixc, i);
		AnthemDescriptorSetEntry dseAccu{ st.descAccuAtt,AT_ACDS_SAMPLER,0 };
		AnthemDescriptorSetEntry dseReveal{ st.descRevealAtt,AT_ACDS_SAMPLER,0 };
		AnthemDescriptorSetEntry dseSolid{ st.descSolidAtt,AT_ACDS_SAMPLER,0 };
		r.drBindDescriptorSetCustomizedGraphics({ dseAccu,dseReveal,dseSolid }, st.pipe, i);
		r.drDraw(st.ixc->getIndexCount(), i);
		r.drEndRenderPass(i);

		r.drEndCommandRecording(i);
	}
}

void mainLoop() {
	static int cur = 0;
	uint32_t imgIdx;
	st.rd.drPrepareFrame(cur, &imgIdx);
	st.rd.drSubmitBufferPrimaryCall(cur, cur);
	st.rd.drPresentFrame(cur, imgIdx);
	cur = 1 - cur;
}

int main() {
	initialize();
	createDescriptor();
	createGeometry();
	updatePushConstant();

	createOpaquePipeline();
	createTransparentPipeline();
	createCompositionPipeline();

	st.rd.registerPipelineSubComponents();
	recordCommands();

	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();
	return 0;
}
/*
float weight =
	max(min(1.0, max(max(color.r, color.g), color.b) * color.a), color.a) *
	clamp(0.03 / (1e-5 + pow(z / 200, 4.0)), 1e-2, 3e3);

// blend func: GL_ONE, GL_ONE
// switch to pre-multiplied alpha and weight
accum = vec4(color.rgb * color.a, color.a) * weight;

// blend func: GL_ZERO, GL_ONE_MINUS_SRC_ALPHA
reveal = color.a;

// fetch pixel information
vec4 accum = texelFetch(rt0, int2(gl_FragCoord.xy), 0);
float reveal = texelFetch(rt1, int2(gl_FragCoord.xy), 0).r;

// blend func: GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA
color = vec4(accum.rgb / max(accum.a, 1e-5), reveal);


*/