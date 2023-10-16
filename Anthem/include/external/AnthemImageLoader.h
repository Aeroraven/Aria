#pragma once
#include "../core/base/AnthemBaseImports.h"

namespace Anthem::External{
    class AnthemImageLoader{
    public:
        bool loadImage(const char* path,uint32_t* width,uint32_t* height,uint32_t* channels,uint8_t** data);
        bool freeImage(uint8_t* data);
    };
}