#include "../../../include/components/postprocessing/AnthemSimpleBlur.h"
namespace Anthem::Components::Postprocessing {
	void AnthemSimpleBlur::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "simpblur\\sblur.";
			st += x;
			st += ".hlsl.spv";
			return st;
			};
		shaderPath.fragmentShader = getShader("frag");
		shaderPath.vertexShader = getShader("vert");
		rd->createShader(&shader, &shaderPath);
	}
	AnthemSimpleBlur::AnthemSimpleBlur(AnthemSimpleToyRenderer* p, uint32_t cmdCopies) :
		AnthemPostprocessPass(p, cmdCopies) {
	}
}