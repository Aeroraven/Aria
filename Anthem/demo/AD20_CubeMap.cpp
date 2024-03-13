#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"
#include "../include/core/drawing/buffer/impl/AnthemVertexBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemInstancingVertexBufferImpl.h"
#include "../include/core/drawing/image/AnthemImage.h"
#include "../include/core/drawing/image/AnthemImageCubic.h"
#include "../include/core/drawing/buffer/impl/AnthemShaderStorageBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemPushConstantImpl.h"

using namespace Anthem::Components::Camera;
using namespace Anthem::External;
using namespace Anthem::Core;


#define ATRANGE(x,y) (std::views::iota((x),(y)))

struct Stage {
	// Core
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);

	// Geometry
	AnthemVertexBufferImpl<AtAttributeVecf<4>>* sbox;
	AnthemIndexBuffer* ix;

	// Texture & Viewport
	AnthemDescriptorPool* descTex;
	AnthemPushConstant* pconst;
	AnthemImageCubic* tex;

	// Display
	AnthemRenderPass* pass;
	AnthemGraphicsPipeline* pipe;
	AnthemSwapchainFramebuffer* fb;
	AnthemDepthBuffer* depth;
	AnthemShaderFilePaths path;
	AnthemShaderModule* shader;

	AnthemRenderPassSetupOption ropt;
	AnthemGraphicsPipelineCreateProps copt;

	// Const
	constexpr static const int inFlight = 2;
}st;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR_HLSL);
	st += "texcube\\texcube.";
	st += x;
	st += ".hlsl.spv";
	return st;
}

inline std::string getSkyboxTex(auto x) {
	std::string st(ANTH_ASSET_DIR);
	st += "skybox\\";
	st += x;
	st += ".jpg";
	return st;
}

void initialize() {
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
	int rdH, rdW;
	st.rd.exGetWindowSize(rdH, rdW);
	st.camera.specifyFrustum((float)AT_PI / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	st.camera.specifyPosition(0, 0, 0);
	st.camera.specifyFrontEyeRay(0, 0, 1);
}

void createGeometry() {
	const std::vector<std::array<float, 4>> vertices = {
		{-1,-1,-1,1},{1,-1,-1,1},{1,1,-1,1},{-1,1,-1,1},
		{-1,-1,1,1},{1,-1,1,1},{1,1,1,1},{-1,1,1,1}
	};
	const std::vector<uint32_t> indices = {
		0,1,2,2,3,0,
		4,5,6,6,7,4,
		0,3,7,7,4,0,
		1,2,6,6,5,1,
		0,1,5,5,4,0,
		3,2,6,6,7,3
	};
	st.rd.createVertexBuffer(&st.sbox);
	st.rd.createIndexBuffer(&st.ix);
	st.ix->setIndices(indices);
	st.sbox->setTotalVertices(8);
	for (auto i : ATRANGE(0, 7))st.sbox->insertData(i, vertices[i]);
}

void createTextures() {
	std::array<uint8_t*, 6> rawData;
	AnthemImageLoader loader;
	for (auto i : ATRANGE(0, 6)) {

	}
}


int main() {
	ANTH_LOGI("Hello World!");
	return 0;
}