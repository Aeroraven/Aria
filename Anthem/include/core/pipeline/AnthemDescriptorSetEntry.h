#pragma once
#include "../base/AnthemBaseImports.h"
#include "AnthemShaderModule.h"
#include "../drawing/AnthemDescriptorPool.h"

namespace Anthem::Core{
    enum AnthemDescriptorSetEntrySourceType{
        AT_ACDS_UNDEFINED = 0,
        AT_ACDS_UNIFORM_BUFFER = 1,
        AT_ACDS_SAMPLER = 2,
        AT_ACDS_SHADER_STORAGE_BUFFER = 3,
        AT_ACDS_STORAGE_IMAGE = 4,
    };
    struct AnthemDescriptorSetEntry{
        AnthemDescriptorPool* descPool = nullptr;
        AnthemDescriptorSetEntrySourceType descSetType = AT_ACDS_UNDEFINED;
        uint32_t inTypeIndex = 0;
    };
}