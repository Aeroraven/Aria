#pragma once
#include "AnthemVertexBufferImpl.h"

namespace Anthem::Core {
    template<typename... AVAOTpDesc>
    class AnthemInstancingVertexBufferImpl;

    template<typename... AttrTp, uint32_t... AttrSz>
    class AnthemInstancingVertexBufferImpl<AnthemVAOAttrDesc<AttrTp, AttrSz>...> :public AnthemVertexBufferImpl<AnthemVAOAttrDesc<AttrTp, AttrSz>...> {
    public:
        bool virtual getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc) override {
            ANTH_ASSERT(desc, "Description is nullptr");
            desc->binding = 0;
            desc->stride = this->singleVertexSize;
            desc->inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            return true;
        }
    };

}