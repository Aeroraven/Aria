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

using namespace Anthem::External;
using namespace Anthem::Core;
using namespace Anthem::Components::Camera;

struct Stage {
	//General
	AnthemSimpleToyRenderer renderer;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
	AnthemConfig config;

	//Cloth
	AnthemDescriptorPool* descSsbo;
	AnthemShaderStorageBufferImpl<
		AtBufVecd4f<1>, //Pos
		AtBufVecd4f<1>, //Vel
	>* ssbo;
	AnthemIndexBuffer* ix;

	//Compute
	AnthemShaderFilePaths compShaderPath;
	AnthemShaderModule* compShader;
	AnthemComputePipeline* compPipe;

	//Graphics
	AnthemShaderFilePaths shaderPath;
	AnthemShaderModule* shader;
	AnthemGraphicsPipeline* dispPipe;
	AnthemRenderPass* pass;
	AnthemFramebuffer* framebuffer;

	//Consts
	const int clothWidth = 64;
}stage;

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "compcl\\compcl.";
	st += x;
	st += ".spv";
	return st;
}

void initialize() {
	stage.renderer.setConfig(&stage.config);
	stage.renderer.initialize();
	int rdH, rdW;
	stage.renderer.exGetWindowSize(rdH, rdW);
	stage.camera.specifyFrustum((float)AT_PI / 2.0f, 0.1f, 500.0f, 1.0f * rdW / rdH);
	stage.camera.specifyPosition(0, 0, -2);
}

int main() {
	ANTH_LOGI("Hello World");
	return 0;
}