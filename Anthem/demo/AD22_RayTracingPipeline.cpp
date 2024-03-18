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
#include "../include/components/utility/AnthemSimpleModelIntegrator.h"

using namespace Anthem::Components::Performance;
using namespace Anthem::Components::Utility;
using namespace Anthem::Components::Camera;
using namespace Anthem::External;
using namespace Anthem::Core;


struct Stage {
	AnthemSimpleToyRenderer rd;
	AnthemConfig cfg;

	// Acceleration Structs
	AnthemBottomLevelAccStruct* blas = nullptr;
	AnthemAccStructGeometry* blasGeo = nullptr;
	AnthemTopLevelAccStruct* tlas = nullptr;
	AnthemAccStructInstance* tlasInst = nullptr;

	// Storage Image
	AnthemImage* storageImage = nullptr;
	uint32_t imgHeight = 0;
	uint32_t imgWidth = 0;
}st;

void initialize() {
	st.rd.setConfig(&st.cfg);
	st.rd.initialize();
}

void createAccelerationStruct(){
	// Bottom-level AS
	std::vector<float> vertex = {
		 1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 0.0f, -1.0f, 0.0f
	};
	std::vector<uint32_t> index = { 0,1,2 };
	std::vector<float> transform = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};
	st.rd.createRayTracingGeometry(&st.blasGeo, 3, vertex, index, transform);
	st.rd.createBottomLevelAS(&st.blas);
	st.blas->addGeometry({ st.blasGeo });
	st.blas->buildBLAS();

	// Top-level AS
	st.rd.createRayTracingInstance(&st.tlasInst, st.blas, transform);
	st.rd.createTopLevelAS(&st.tlas);
	st.tlas->addInstance({ st.tlasInst });
	st.tlas->buildTLAS();
}

void createStorageImage() {
	st.rd.getSwapchainImageExtent(&st.imgWidth, &st.imgHeight);
	st.rd.createTexture(&st.storageImage, nullptr, nullptr,
		st.imgWidth, st.imgHeight, 4, 0, false, false, AT_IF_SWAPCHAIN, -1, true, AT_IU_RAYTRACING_DEST);
	st.storageImage->toGeneralLayout();
}


int main() {
	constexpr static const char* x = "a""b";
	ANTH_LOGI(x);
	initialize();
	st.rd.finalize();
	return 0;
}