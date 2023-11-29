#pragma once
#include "AnthemVertexStageBuffer.h"

namespace Anthem::Core{
    class AnthemIndexBuffer : public AnthemVertexStageBuffer{
    private:
        std::vector<uint32_t> indices;
    public:
        bool virtual setIndices(std::vector<uint32_t> indices);
        bool virtual createBuffer() override;
        uint32_t virtual calculateBufferSize() override;
        void virtual getRawBufferData(void** dataDst) override;
        uint32_t getIndexCount();
    };
}