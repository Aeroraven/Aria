#include "../../include/external/AnthemGLTFLoader.h"

namespace Anthem::External{
    bool AnthemGLTFLoader::loadModel(const char* path){
        std::string err,warn;
        bool ret = loader.LoadASCIIFromFile(&model,&err,&warn,path);
        if(!warn.empty()){
            ANTH_LOGW(warn.c_str());
        }
        if(!err.empty()){
            ANTH_LOGE(err.c_str());
        }
        if(!ret){
            ANTH_LOGE("Failed to load gltf model");
            return false;
        }
        this->modelPath = path;
        auto end = this->modelPath.rfind("\\");
        this->modelDirectory = this->modelPath.substr(0,end);
        ANTH_LOGI("Model Dir=",this->modelDirectory);
        return true;
    }
    bool AnthemGLTFLoader::parseModel(AnthemGLTFLoaderParseConfig config,std::vector<AnthemGLTFLoaderParseResult>& result, AnthemGLTFLoaderTexParseResult* texResult){
        int totalResultSize = 0;
        for (auto subMesh : model.meshes) {
            totalResultSize += subMesh.primitives.size();
        }
        result.reserve(totalResultSize);
        result.resize(totalResultSize);

        const auto& accessor = model.accessors;
        const auto& materials = model.materials;
        const auto& textures = model.textures;
        const auto specifyStride = [](int tp,int& dimOutput,std::string& tpOutput)->int{
            if(tp==TINYGLTF_TYPE_SCALAR){
                dimOutput = 1;
                tpOutput = "short";
                return 2;
            }else if(tp==TINYGLTF_TYPE_VEC2){
                dimOutput = 2;
                tpOutput = "float";
                return 4;
            }else if(tp==TINYGLTF_TYPE_VEC3){   
                dimOutput = 3;
                tpOutput = "float";
                return 4;
            }else if(tp==TINYGLTF_TYPE_VEC4){
                dimOutput = 4;
                tpOutput = "float";
                return 4;
            }else if(tp==TINYGLTF_TYPE_MAT4){
                dimOutput = 16;
                tpOutput = "float";
                return 4;
            }
            ANTH_LOGE("Unknown Format");
            return 0;
        };
        uint32_t curResultSlot = 0;
        if (texResult != nullptr) {
            auto& q = *texResult;
            q.tex.reserve(model.images.size());
            q.tex.resize(model.images.size());
            q.channels.reserve(model.images.size());
            q.channels.resize(model.images.size());
            q.height.reserve(model.images.size());
            q.height.resize(model.images.size());
            q.width.reserve(model.images.size());
            q.width.resize(model.images.size());
            for (auto i : AT_RANGE2(model.images.size())) {
                q.tex[i] = std::move(model.images[i].image);
                q.width[i] = model.images[i].width;
                q.height[i] = model.images[i].height;
                q.channels[i] = model.images[i].component;
                ANTH_ASSERT(model.images[i].bits == 8, "Bits != 8");
            }
        }
        for(int i=0;i<model.meshes.size();i++){
            ANTH_LOGI("Parsing Mesh",i);
            auto& curMesh = model.meshes.at(i);
            auto k = 0;
            for (auto & curPrimitive : curMesh.primitives) {
                ANTH_LOGI("Parsing Submesh", k++);
                auto& curResult = result.at(curResultSlot++);
                
                // Index Accessor
                auto& indexAccessor = accessor.at(curPrimitive.indices);
                auto indexOffset = indexAccessor.byteOffset;
                auto indexCount = indexAccessor.count;
                auto& indexBufferView = model.bufferViews.at(indexAccessor.bufferView);
                auto stride = indexBufferView.byteStride;
                bufferParse<uint32_t, uint16_t>(indexAccessor.bufferView, 2, curResult.indices, indexOffset, indexCount);

                // Material Entry
                auto& materialEntry = materials.at(curPrimitive.material);
                auto& materialIndexPbrBase = materialEntry.pbrMetallicRoughness.baseColorTexture.index;
                auto& materialIndexNormal = materialEntry.normalTexture.index;

                curResult.pbrBaseTexId = -1;
                curResult.pbrNormalTexId = -1;
                if (materialIndexPbrBase != -1) {
                    auto imageIdx = textures.at(materialIndexPbrBase).source;
                    std::string dest;
                    tinygltf::URIDecode(model.images.at(imageIdx).uri, &dest, nullptr);
                    curResult.pbrBaseColorTexPath = dest;
                    curResult.pbrBaseTexId = imageIdx;
                }
                if (materialIndexNormal != -1) {
                    auto imageIdx = textures.at(materialIndexNormal).source;
                    curResult.pbrNormalTexId = imageIdx;
                }

                curResult.pbrBaseColorFactor = materialEntry.pbrMetallicRoughness.baseColorFactor;
                // Attribute Accessor
                for (auto& attr : curPrimitive.attributes) {
                    auto& attrAccessor = accessor.at(attr.second);
                    auto attrAccessorOffset = attrAccessor.byteOffset;
                    auto attrAccessorCount = attrAccessor.count;
                    auto& attrBufferView = model.bufferViews.at(attrAccessor.bufferView);
                    auto& attrBuffer = model.buffers.at(attrBufferView.buffer);
                    auto& attrBufferOffset = attrBufferView.byteOffset;
                    auto& attrBufferLength = attrBufferView.byteLength;
                    auto& attrBufferStride = attrBufferView.byteStride;
                    auto& attrName = attr.first;

                    
                    if (attrName == config.positionPrimitiveName) {
                        curResult.positionPrimitiveType = attrAccessor.type;
                        specifyStride(attrAccessor.type, curResult.positionDim, curResult.positionPrimitiveType);
                        attrAccessorCount *= curResult.positionDim;
                        bufferParse<float, float>(attrAccessor.bufferView, 4, curResult.positions,
                            attrAccessorOffset, attrAccessorCount);
                    }
                    else if (attrName == config.normalPrimitiveName) {
                        curResult.normalPrimitiveType = attrAccessor.type;
                        specifyStride(attrAccessor.type, curResult.normalDim, curResult.normalPrimitiveType);
                        attrAccessorCount *= curResult.normalDim;
                        bufferParse<float, float>(attrAccessor.bufferView, 4, curResult.normals,
                            attrAccessorOffset, attrAccessorCount);
                    }
                    else if (attrName == config.texCoordPrimitiveName) {
                        curResult.texCoordPrimitiveType = attrAccessor.type;
                        specifyStride(attrAccessor.type, curResult.texCoordDim, curResult.texCoordPrimitiveType);
                        attrAccessorCount *= curResult.texCoordDim;
                        bufferParse<float, float>(attrAccessor.bufferView, 4, curResult.texCoords,
                            attrAccessorOffset, attrAccessorCount);
                    }
                    else if (attrName == config.tangentPrimitiveName) {
                        curResult.tangentPrimitiveType = attrAccessor.type;
                        specifyStride(attrAccessor.type, curResult.tangentDim, curResult.tangentPrimitiveType);
                        attrAccessorCount *= curResult.tangentDim;
                        bufferParse<float, float>(attrAccessor.bufferView, 4, curResult.tangents,
                            attrAccessorOffset, attrAccessorCount);

                        if (curResult.tangents.size() == 0) {
                            ANTH_LOGE("ww");
                        }
                    }
                }
                curResult.basePath = this->modelDirectory + "/";
                if (curResult.positions.size()) {
                    curResult.numVertices = curResult.positions.size() / 3;
                }
                ANTH_LOGI("Parse done.");
                ANTH_LOGI("Position Primitive Type:", curResult.positionPrimitiveType, "Dimension:", curResult.positionDim, "Length", curResult.positions.size());
                ANTH_LOGI("Normal Primitive Type:", curResult.normalPrimitiveType, "Dimension:", curResult.normalDim, "Length", curResult.normals.size());
                ANTH_LOGI("TexCoord Primitive Type:", curResult.texCoordPrimitiveType, "Dimension:", curResult.texCoordDim, "Length", curResult.texCoords.size());
                ANTH_LOGI("Tangent Primitive Type:", curResult.tangentPrimitiveType, "Dimension:", curResult.tangentDim, "Length", curResult.tangents.size());
                ANTH_LOGI("Tex Path", curResult.pbrBaseColorTexPath);
            }
            
        }
        ANTH_LOGI("Parsed ", totalResultSize , " Models");
        return true;
    }
}