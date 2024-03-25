#include "../../../include/components/postprocessing/AnthemTAA.h"
namespace Anthem::Components::Postprocessing {
    void AnthemTAA::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "taa\\taa.";
			st += x;
			st += ".hlsl.spv";
			return st;
		};
		shaderPath.fragmentShader = getShader("frag");
		shaderPath.vertexShader = getShader("vert");
		rd->createShader(&shader, &shaderPath);
    }
	AnthemTAA::AnthemTAA(AnthemSimpleToyRenderer* p, uint32_t cmdCopies):
	AnthemPostprocessPass(p, cmdCopies){
	}
}