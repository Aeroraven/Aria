#include "../include/core/renderer/AnthemSimpleToyRenderer.h"

using namespace Anthem::Core;

struct Stage {
	const int inFlight = 2;
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;

	AnthemDepthBuffer* depth;
	AnthemShaderFilePaths path;
	AnthemShaderModule* shader;
	AnthemGraphicsPipeline* pipe;
	AnthemRenderPass* pass;
	AnthemSwapchainFramebuffer* fb;

	AnthemRenderPassSetupOption ropt;
	AnthemGraphicsPipelineCreateProps copt;
	
}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "mesh\\mesh.";
	st += x;
	st += ".hlsl.spv";
	return st;
}


void initialize() {
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
}


void prepareStage() {
	st.rd.createDepthBuffer(&st.depth,false);

	st.path.fragmentShader = getShader("frag");
	st.path.meshShader = getShader("mesh");
	st.path.taskShader = getShader("task");
	st.rd.createShader(&st.shader, &st.path);

	st.ropt.msaaType = AT_ARPMT_NO_MSAA;
	st.ropt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	st.rd.setupRenderPass(&st.pass, &st.ropt, st.depth);

	st.rd.createSwapchainImageFramebuffers(&st.fb, st.pass, st.depth);

	st.copt.emptyVertexStage = true;
	st.copt.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
	st.rd.createGraphicsPipelineCustomized(&st.pipe, {}, {}, st.pass, st.shader, nullptr, &st.copt);
}

void recordCommand() {
	auto& r = st.rd;
	for (int i = 0; i < st.inFlight; i++) {
		r.drStartCommandRecording(i);
		r.drStartRenderPass(st.pass, st.fb->getFramebufferObjectUnsafe(i), i, false);
		r.drSetViewportScissorFromSwapchain(i);
		r.drBindGraphicsPipeline(st.pipe, i);
		r.drDrawMesh(1, 1, 1, i);
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
	prepareStage();
	st.rd.registerPipelineSubComponents();
	recordCommand();
	st.rd.setDrawFunction(mainLoop);
	st.rd.startDrawLoopDemo();
	st.rd.finalize();
	return 0;
}