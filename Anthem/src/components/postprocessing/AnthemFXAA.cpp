#include "../../../include/components/postprocessing/AnthemFXAA.h"
namespace Anthem::Components::Postprocessing {
    void AnthemFXAA::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "fxaa\\fxaa.";
			st += x;
			st += ".hlsl.spv";
			return st;
		};
		shaderPath.fragmentShader = getShader("frag");
		shaderPath.vertexShader = getShader("vert");
		rd->createShader(&shader, &shaderPath);
    }
	AnthemFXAA::AnthemFXAA(AnthemSimpleToyRenderer* p, uint32_t cmdCopies):
	AnthemPostprocessPass(p, cmdCopies){
	}
}