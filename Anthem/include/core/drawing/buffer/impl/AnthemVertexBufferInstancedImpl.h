#pragma once
#include "../../../base/AnthemBaseImports.h"
#include "../AnthemVertexBuffer.h"

namespace Anthem::Core {
    template<typename Tp, uint32_t Sz>
        requires TmplDefs::ATpIsdValidAttr<Tp, Sz>
    struct AnthemVAOInstancedAttrDesc;

    template<uint32_t Sz>
    using AtAttributeInstVecf = AnthemVAOInstancedAttrDesc<float, Sz>;

    template<typename... Tp>
    struct AnthemVertexBufferInstancedAttrs;

    template<typename... Tp>
    struct AnthemVertexBufferNonInstancedAttrs;

    template<typename... Tp>
    class AnthemVertexBufferInstancedImpl;

    template<typename... AttrVertexTp, uint32_t... AttrVertexSz, typename... AttrInstTp, uint32_t... AttrInstSz>
    class AnthemVertexBufferInstancedImpl < AnthemVertexBufferNonInstancedAttrs< AnthemVAOInstancedAttrDesc< AttrVertexTp, AttrVertexSz>...>,
        AnthemVertexBufferInstancedImpl< AnthemVAOInstancedAttrDesc< AttrInstTp, AttrInstSz>...>>:public virtual AnthemVertexBuffer{

    private:
        static constexpr std::array<uint32_t, sizeof...(AttrVertexSz)> vertexBufDims = { AttrVertexSz ... };
        static constexpr std::array<uint32_t, sizeof...(AttrVertexTp)> vertexBufSizes = { sizeof(AttrVertexTp)... };
        static constexpr std::array<uint32_t, sizeof...(AttrInstSz)> instancedBufDims = { AttrInstSz ... };
        static constexpr std::array<uint32_t, sizeof...(AttrInstTp)> instancedBufSizes = { sizeof(AttrInstTp)... };

        uint32_t vertexPartStride = 0;
        uint32_t instancePartStride = 0;
        uint32_t attrBindPointVertex[sizeof...(AttrVertexTp)];
        uint32_t attrBindPointInstanced[sizeof...(AttrInstTp)];

    public:
        AnthemVertexBufferImpl() {
            this->vertexPartStride = 0;
            for (auto i = 0; i < sizeof...(AttrVertexSz); i++) {
                this->vertexPartStride += this->vertexBufDims[i] * this->vertexBufSizes[i];
                attrBindPointVertex[i] = i;
            }
            this->instancePartStride = 0;
            for (auto i = 0; i < sizeof...(AttrInstSz); i++) {
                this->instancePartStride += this->instancedBufDims[i] * this->instancedBufSizes[i];
                attrBindPointInstanced[i] = i + sizeof...(AttrVertexTp);
            }
        }


        bool virtual getInputBindingDescriptionInternal(VkVertexInputBindingDescription* desc, uint32_t bindLoc) override {
            return true;
        }
        bool virtual getInputAttrDescriptionInternal(std::vector<VkVertexInputAttributeDescription>* desc, uint32_t bindLoc) override {
            return true;
        }
        bool virtual prepareVertexInputInfo(VkPipelineVertexInputStateCreateInfo* info, uint32_t bindLoc) override {
            return true;
        }
        bool virtual updateLayoutSpecification(AnthemVertexStageLayoutSpec* spec, uint32_t bindLoc) override {
            return true;
        }
    };
}