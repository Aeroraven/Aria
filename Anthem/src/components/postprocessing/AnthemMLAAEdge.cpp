#include "../../../include/components/postprocessing/AnthemMLAAEdge.h"
namespace Anthem::Components::Postprocessing {
	void AnthemMLAAEdge::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "mlaa\\mlaa.edge.";
			st += x;
			st += ".hlsl.spv";
			return st;
			};
		shaderPath.fragmentShader = getShader("frag");
		shaderPath.vertexShader = getShader("vert");
		rd->createShader(&shader, &shaderPath);
	}
	AnthemMLAAEdge::AnthemMLAAEdge(AnthemSimpleToyRenderer* p, uint32_t cmdCopies,uint32_t searchRanges) :
		AnthemPostprocessPass(p, cmdCopies) {
		this->searchRange = searchRanges;
	}
	
}