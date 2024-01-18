#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"
#include "../include/core/drawing/buffer/impl/AnthemVertexBufferInstancedImpl.h"


using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::External;
using namespace Anthem::Components::Camera;

int main() {
	AnthemVertexBufferInstancedImpl<
		AnthemVertexBufferNonInstancedAttrs< AtAttributeInstVecf<4>, AtAttributeInstVecf<3>>,
		AnthemVertexBufferInstancedImpl< AtAttributeInstVecf<4>>
	>();
	return 0;
}