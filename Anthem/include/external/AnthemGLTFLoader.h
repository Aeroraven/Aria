#include "../external_ref/tinygltf/tiny_gltf.h"
#include "../core/base/AnthemBaseImports.h"
#include "../core/utils/AnthemUtlSimpleModelStruct.h"

namespace Anthem::External{
    struct AnthemGLTFLoaderParseConfig{
        std::string positionPrimitiveName = "POSITION";
        std::string normalPrimitiveName = "NORMAL";
        std::string texCoordPrimitiveName = "TEXCOORD_0";
    };

    struct AnthemGLTFLoaderParseResult: public virtual Anthem::Core::AnthemUtlSimpleModelStruct {};

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