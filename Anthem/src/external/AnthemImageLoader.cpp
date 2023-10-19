#include "../../include/external/AnthemImageLoader.h"
#include "../../include/external_ref/tinygltf/stb_image.h"

namespace Anthem::External{
    bool AnthemImageLoader::freeImage(uint8_t* data){
        stbi_image_free(data);
        return true;
    }
    bool AnthemImageLoader::loadImage(const char* path,uint32_t* width,uint32_t* height,uint32_t* channels,uint8_t** data){
        *data = stbi_load(path,(int*)width,(int*)height,(int*)channels,STBI_rgb_alpha);
        ANTH_ASSERT(*data != nullptr,"Failed to load image");
        //Convert to RGBA
        if (*data != nullptr){
            *channels = 4;
            uint32_t size = (*width)*(*height)*(*channels);
            uint8_t* newData = new uint8_t[size];
            for (uint32_t i = 0; i < size; i+=4){
                newData[i] = (*data)[i];
                newData[i+1] = (*data)[i+1];
                newData[i+2] = (*data)[i+2];
                newData[i+3] = 0;
            }
            delete[] *data;
            *data = newData;
            
        }
        return true;
    }
}