#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/utils/AnthemUtlSimpleModelStruct.h"
#include "../../core/renderer/AnthemSimpleToyRenderer.h"
#include "../../core/drawing/buffer/acceleration/AnthemAccelerationStruct.h"
#include "../../core/drawing/buffer/acceleration/AnthemAccStructGeometry.h"
#include "../../core/drawing/buffer/acceleration/AnthemAccStructInstance.h"
#include "../../core/drawing/buffer/acceleration/AnthemBottomLevelAccStruct.h"
#include "../../core/drawing/buffer/acceleration/AnthemTopLevelAccStruct.h"



namespace Anthem::Components::Utility {
	using namespace Anthem::Core;

	struct AnthemSimpleModelIntegratorRayTracingStructs {
		AnthemShaderStorageBufferImpl<AtBufVecd4f<1>>* posBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecd4f<1>>* normalBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecd2f<1>>* texBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecdUint<1>>* indexBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecdUint<1>>* offsetBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecdUint<1>>* lightIndexBuffer;

		AnthemDescriptorPool* descPos;
		AnthemDescriptorPool* descNormal;
		AnthemDescriptorPool* descTex;
		AnthemDescriptorPool* descIndex;
		AnthemDescriptorPool* descOffset;
		AnthemDescriptorPool* descLightIdx;

		std::vector<AnthemAccStructGeometry*> asGeo;
		std::vector<AnthemBottomLevelAccStruct*> blasObj;

		AnthemAccStructInstance* asInst;
		AnthemTopLevelAccStruct* tlasObj;
	};

	class AnthemSimpleModelIntegrator {
	private:
		AnthemVertexBufferImpl<AtAttributeVecf<4>, AtAttributeVecf<4>, AtAttributeVecf<4>>* vx = nullptr;
		AnthemIndexBuffer* ix = nullptr;
		AnthemIndirectDrawBuffer* indirect = nullptr;


		// Raytracing

		AnthemShaderStorageBufferImpl<AtBufVecd4f<1>>* posBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecd4f<1>>* normalBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecd2f<1>>* texBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecdUint<1>>* indexBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecdUint<1>>* offsetBuffer;
		AnthemShaderStorageBufferImpl<AtBufVecdUint<1>>* lightIndexBuffer;

		AnthemDescriptorPool* descPos;
		AnthemDescriptorPool* descNormal;
		AnthemDescriptorPool* descTex;
		AnthemDescriptorPool* descIndex;
		AnthemDescriptorPool* descOffset;
		AnthemDescriptorPool* descLightIdx;

		std::vector<AnthemAccStructGeometry*> asGeo;
		std::vector<AnthemBottomLevelAccStruct*> blasObj;

		AnthemAccStructInstance* asInst;
		AnthemTopLevelAccStruct* tlasObj;

		float totalLightAreas = 0;
		uint32_t totalLightFaces = 0;

	public:
		bool loadModel(AnthemSimpleToyRenderer* renderer,std::vector<AnthemUtlSimpleModelStruct> model, uint32_t cpuJobs);
		bool loadModelRayTracing(AnthemSimpleToyRenderer* renderer, const std::vector<AnthemUtlSimpleModelStruct>& model, const std::set<int> lightIds);
		
		AnthemVertexBuffer* getVertexBuffer();
		AnthemIndexBuffer* getIndexBuffer();
		AnthemIndirectDrawBuffer* getIndirectBuffer();

		AnthemSimpleModelIntegratorRayTracingStructs getRayTracingParsedResult();
		float getLightAreas();
		uint32_t getLightFaces();
	};
}