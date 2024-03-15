#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/components/performance/AnthemFrameRateMeter.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"
#include "../include/core/drawing/buffer/impl/AnthemVertexBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemInstancingVertexBufferImpl.h"
#include "../include/core/drawing/image/AnthemImage.h"
#include "../include/core/drawing/image/AnthemImageCubic.h"
#include "../include/core/drawing/buffer/impl/AnthemShaderStorageBufferImpl.h"
#include "../include/core/drawing/buffer/impl/AnthemPushConstantImpl.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Camera;
using namespace Anthem::External;
using namespace Anthem::Core;

struct Stage {
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);


};

int main() {
	ANTH_LOGI("Hello World");
	return 0;
}