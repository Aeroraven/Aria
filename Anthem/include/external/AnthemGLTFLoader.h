#include "../external_ref/tinygltf/tiny_gltf.h"
#include "../core/base/AnthemBaseImports.h"
#include "../core/utils/AnthemUtlSimpleModelStruct.h"

namespace Anthem::External{
    struct AnthemGLTFLoaderParseConfig{
        std::string positionPrimitiveName = "POSITION";
        std::string normalPrimitiveName = "NORMAL";
        std::string texCoordPrimitiveName = "TEXCOORD_0";
        std::string tangentPrimitiveName = "TANGENT";
    };

    struct AnthemGLTFLoaderParseResult: public virtual Anthem::Core::AnthemUtlSimpleModelStruct {};
    struct AnthemGLTFLoaderTexParseResult : public virtual Anthem::Core::AnthemUtlSimpleTexImageStruct {};

    class AnthemGLTFLoader{
    private:
        std::string modelPath = "";
        std::string modelDirectory = "";
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
    public:
        template<typename T,typename U>
        bool bufferParse(int bufferIdx,int stride,std::vector<T>& result,int accessorOffset,int accessorCount){
            const auto& bufferView = model.bufferViews.at(bufferIdx);
            const auto& buffer = model.buffers.at(bufferView.buffer);
            const auto& bufferData = buffer.data.data();
            const auto& bufferOffset = bufferView.byteOffset;
            const auto& bufferLength = bufferView.byteLength;
            if (std::is_same_v<T,U> && sizeof(T) == stride) {
                int elems = std::min(accessorCount, static_cast<int>(bufferLength / stride));
                result.reserve(elems);
                result.resize(elems);
                memcpy(result.data(), bufferData + bufferOffset + accessorOffset, stride * elems);
            }
            else {
                for (int i = 0, j = 0; j < accessorCount && i < bufferLength; j++, i += stride) {
                    result.push_back(static_cast<T>(*((U*)(bufferData + bufferOffset + i + accessorOffset))));
                }
            }
            
            return true;
        }
        bool loadModel(const char* path);
        bool parseModel(AnthemGLTFLoaderParseConfig config,std::vector<AnthemGLTFLoaderParseResult>& result, AnthemGLTFLoaderTexParseResult* texResult=nullptr);
    };
}