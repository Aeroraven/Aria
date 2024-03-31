#include "../../../include/components/postprocessing/AnthemPostIdentity.h"
namespace Anthem::Components::Postprocessing {
	void AnthemPostIdentity::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "identity\\id.";
			st += x;
			st += ".hlsl.spv";
			return st;
			};
		shaderPath.fragmentShader = getShader("frag");
		shaderPath.vertexShader = getShader("vert");
		rd->createShader(&shader, &shaderPath);
	}
	AnthemPostIdentity::AnthemPostIdentity(AnthemSimpleToyRenderer* p, uint32_t cmdCopies) :
		AnthemPostprocessPass(p, cmdCopies) {
	}
}