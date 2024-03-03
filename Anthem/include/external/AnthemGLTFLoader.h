#include "../external_ref/tinygltf/tiny_gltf.h"
#include "../core/base/AnthemBaseImports.h"

namespace Anthem::External{
    struct AnthemGLTFLoaderParseConfig{
        std::string positionPrimitiveName = "POSITION";
        std::string normalPrimitiveName = "NORMAL";
        std::string texCoordPrimitiveName = "TEXCOORD_0";
    };

    struct AnthemGLTFLoaderParseResult{
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

    class AnthemGLTFLoader{
    private:
        std::string modelPath = "";
        std::string modelDirectory = "";
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
    public:
        template<typename T,typename U>
        bool bufferParse(int bufferIdx,int stride,std::vector<T>& result){
            const auto& bufferView = model.bufferViews.at(bufferIdx);
            const auto& buffer = model.buffers.at(bufferView.buffer);
            const auto& bufferData = buffer.data.data();
            const auto& bufferOffset = bufferView.byteOffset;
            const auto& bufferLength = bufferView.byteLength;
            for(int i=0;i<bufferLength;i+=stride){
                result.push_back(static_cast<T>(*((U*)(bufferData + bufferOffset + i))));
            }
            return true;
        }
        bool loadModel(const char* path);
        bool parseModel(AnthemGLTFLoaderParseConfig config,std::vector<AnthemGLTFLoaderParseResult>& result);
    };
}