#include "../../../include/components/postprocessing/AnthemMLAABlend.h"
namespace Anthem::Components::Postprocessing {
	void AnthemMLAABlend::prepareShader() {
		auto getShader = [&](auto x) {
			std::string st(ANTH_SHADER_DIR_HLSL);
			st += "mlaa\\mlaa.blend.";
			st += x;
			st += ".hlsl.spv";
			return st;
			};
		shaderPath.fragmentShader = getShader("frag");
		shaderPath.vertexShader = getShader("vert");
		rd->createShader(&shader, &shaderPath);
	}
	AnthemMLAABlend::AnthemMLAABlend(AnthemSimpleToyRenderer* p, uint32_t cmdCopies,uint32_t searchRanges) :
		AnthemPostprocessPass(p, cmdCopies) {
		this->searchRange = searchRanges;
	}
	void AnthemMLAABlend::prepareBlendTexture(std::string path) {
		int reqSize = searchRange * 4;
		cv::Mat img(reqSize, reqSize, CV_8UC4);
		for (int i = 0; i < 4; i++) {
			float leftHeight = (i == 0 || i == 3) ? 0 : ((i == 1) ? 1: -1);
			for (int j = 0; j < 4; j++) {
				float rightHeight = (j == 0 || j == 3) ? 0 : ((j == 1) ? 1 : -1);
				for (int p = 0; p < searchRange; p++) {
					for (int q = 0; q < searchRange; q++) {
						int cLocY = i * searchRange + p;
						int cLocX = j * searchRange + q;
						
						int lH = leftHeight, rH = rightHeight;
						if (i == 3 && (j == 1 || j == 2)) {
							lH = -1 / rH;
						}
						if (j == 3 && (i == 1 || i == 2)) {
							rH = -1 / lH;
						}
						float center = (p - q) * 1.0f / 2;
						float percent = 0;
						auto& px = img.at<cv::Vec4b>(cLocX, cLocY);
						if (p < q) {
							percent = (1 - 1.0 * (p+0.5) / ((1.0 * p + q + 1.0) / 2)) * lH;
							if (std::abs(percent) > 1)throw;
						}
						else {
							percent = (1 - 1.0 * (q+0.5) / ((1.0 * p + q + 1.0) / 2)) * rH;
							if (std::abs(percent) > 1)throw;
						}

						px[0] = 0; //B
						px[3] = 255;
						px[2] = 255 * AT_CLAMP(-percent, 0, 1); //R
						px[1] = 255 * AT_CLAMP(percent, 0, 1); //G
						if (px[1] != 0 && px[2] != 0) {
							throw;
						}
					}
				}
			}
		}
		cv::imwrite(path, img);
	}
}