#pragma once
#include "../../core/base/AnthemBaseImports.h"
#include "../../core/drawing/buffer/AnthemVertexBuffer.h"
#include "../../core/drawing/buffer/AnthemIndexBuffer.h"
#include "../../core/drawing/buffer/AnthemIndexBuffer.h"
#include "../../core/pipeline/AnthemGraphicsPipeline.h"


namespace Anthem::Components::Geometry {
using namespace Anthem::Core;

	class AnthemBaseGeometry {
		
	protected:
		AnthemVertexBuffer* position = nullptr;
		AnthemIndexBuffer* indices = nullptr;
		AnthemInputAssemblerTopology* topology = nullptr;

		virtual void setPosition(AnthemVertexBuffer* pos) = 0;
		virtual void setIndices(AnthemIndexBuffer* idx) = 0;
		virtual void setTopology(AnthemInputAssemblerTopology topo) = 0;
	};
}
