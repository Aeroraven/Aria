#pragma once
#include "../base/AnthemDefs.h"

namespace Anthem::Core {
	struct AnthemUtlSimpleModelStruct {
        std::vector<float> positions = {};
        std::vector<float> normals = {};
        std::vector<float> texCoords = {};
        std::vector<uint32_t> indices = {};

        std::string positionPrimitiveType = "";
        std::string normalPrimitiveType = "";
        std::string texCoordPrimitiveType = "";

        std::string pbrBaseColorTexPath = "";
        std::string basePath = "";

        uint32_t numVertices = 0;

        int positionDim = 3;
        int normalDim = 3;
        int texCoordDim = 2;
	};
}