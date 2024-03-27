#include "../../../include/components/postprocessing/AnthemMLAAMix.h"
namespace Anthem::Components::Postprocessing {
	void AnthemMLAAMix::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "mlaa\\mlaa.mix.";
			st += x;
			st += ".hlsl.spv";
			return st;
			};
		shaderPath.fragmentShader = getShader("frag");
		shaderPath.vertexShader = getShader("vert");
		rd->createShader(&shader, &shaderPath);
	}
	AnthemMLAAMix::AnthemMLAAMix(AnthemSimpleToyRenderer* p, uint32_t cmdCopies,uint32_t searchRanges) :
		AnthemPostprocessPass(p, cmdCopies) {
		this->searchRange = searchRanges;
	}
	
}