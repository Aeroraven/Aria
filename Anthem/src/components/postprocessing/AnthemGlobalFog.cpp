#include "../../../include/components/postprocessing/AnthemGlobalFog.h"
namespace Anthem::Components::Postprocessing {
	void AnthemGlobalFog::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "globalfog\\gfog.";
			st += x;
			st += ".hlsl.spv";
			return st;
			};
		shaderPath.fragmentShader = getShader("frag");
		shaderPath.vertexShader = getShader("vert");
		rd->createShader(&shader, &shaderPath);
	}
	AnthemGlobalFog::AnthemGlobalFog(AnthemSimpleToyRenderer* p, uint32_t cmdCopies) :
		AnthemPostprocessPass(p, cmdCopies) {
	}
}