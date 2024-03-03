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
    bool AnthemGLTFLoader::parseModel(AnthemGLTFLoaderParseConfig config,std::vector<AnthemGLTFLoaderParseResult>& result){
        result.resize(model.meshes.size());
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
            return 0;
        };
        for(int i=0;i<model.meshes.size();i++){
            ANTH_LOGI("Parsing Mesh",i);
            auto& curMesh = model.meshes.at(i);
            auto& curResult = result.at(i);
            auto& curPrimitive = curMesh.primitives.at(0);
            // Index Accessor
            auto& indexAccessor = accessor.at(curPrimitive.indices);
            auto& indexBufferView = model.bufferViews.at(indexAccessor.bufferView);
            auto stride = indexBufferView.byteStride;
            bufferParse<uint32_t,uint16_t>(indexAccessor.bufferView,2,curResult.indices);

            // Material Entry
            auto& materialEntry = materials.at(curPrimitive.material);
            auto& materialIndex = materialEntry.pbrMetallicRoughness.baseColorTexture.index;
            if(materialIndex!=-1){
                auto imageIdx = textures.at(materialIndex).source;
                curResult.pbrBaseColorTexPath = model.images.at(imageIdx).uri;
            }

            // Attribute Accessor
            for(auto& attr : curPrimitive.attributes){
                auto& attrAccessor = accessor.at(attr.second);
                auto& attrBufferView = model.bufferViews.at(attrAccessor.bufferView);
                auto& attrBuffer = model.buffers.at(attrBufferView.buffer);
                auto& attrBufferOffset = attrBufferView.byteOffset;
                auto& attrBufferLength = attrBufferView.byteLength;
                auto& attrBufferStride = attrBufferView.byteStride;
                auto& attrName = attr.first;
                if(attrName==config.positionPrimitiveName){
                    curResult.positionPrimitiveType = attrAccessor.type;
                    bufferParse<float,float>(attrAccessor.bufferView,4,curResult.positions);
                    specifyStride(attrAccessor.type,curResult.positionDim,curResult.positionPrimitiveType);
                }else if(attrName==config.normalPrimitiveName){
                    curResult.normalPrimitiveType = attrAccessor.type;
                    bufferParse<float,float>(attrAccessor.bufferView,4,curResult.normals);
                    specifyStride(attrAccessor.type,curResult.normalDim,curResult.normalPrimitiveType);
                }else if(attrName==config.texCoordPrimitiveName){
                    curResult.texCoordPrimitiveType = attrAccessor.type;
                    bufferParse<float,float>(attrAccessor.bufferView,4,curResult.texCoords);
                    specifyStride(attrAccessor.type,curResult.texCoordDim,curResult.texCoordPrimitiveType);
                }
            }
            curResult.basePath = this->modelDirectory+"/";
            if (curResult.positions.size()) {
                curResult.numVertices = curResult.positions.size() / 3;
            }
            ANTH_LOGI("Parse done.");
            ANTH_LOGI("Position Primitive Type:",curResult.positionPrimitiveType, "Dimension:",curResult.positionDim,"Length",curResult.positions.size());
            ANTH_LOGI("Normal Primitive Type:",curResult.normalPrimitiveType, "Dimension:",curResult.normalDim,"Length",curResult.normals.size());
            ANTH_LOGI("TexCoord Primitive Type:",curResult.texCoordPrimitiveType, "Dimension:",curResult.texCoordDim,"Length",curResult.texCoords.size());
            ANTH_LOGI("Tex Path",curResult.pbrBaseColorTexPath);
        }
        return true;
    }
}