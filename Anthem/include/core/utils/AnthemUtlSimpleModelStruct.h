#pragma once
#include "../base/AnthemDefs.h"

namespace Anthem::Core {
    struct AnthemUtlSimpleTexImageStruct {
        std::vector<std::vector<uint8_t>> tex;
        std::vector<uint32_t> width;
        std::vector<uint32_t> height;
        std::vector<uint32_t> channels;
    };
	struct AnthemUtlSimpleModelStruct {
        std::vector<float> positions = {};
        std::vector<float> normals = {};
        std::vector<float> texCoords = {};
        std::vector<float> tangents = {};
        std::vector<uint32_t> indices = {};
        
        int pbrBaseTexId = -1;
        int pbrNormalTexId = 1;

        std::string positionPrimitiveType = "";
        std::string normalPrimitiveType = "";
        std::string texCoordPrimitiveType = "";
        std::string tangentPrimitiveType = "";

        std::string pbrBaseColorTexPath = "";
        std::vector<double> pbrBaseColorFactor = {0,0,0,1};
        std::string basePath = "";

        uint32_t numVertices = 0;

        int positionDim = 3;
        int normalDim = 3;
        int texCoordDim = 2;
        int tangentDim = 4;
	};
}