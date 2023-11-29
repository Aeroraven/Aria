#pragma once
#include "../../../base/AnthemBaseImports.h"
#include "../AnthemUniformBuffer.h"

namespace Anthem::Core {
    template<typename Tp, uint32_t Rk, uint32_t Sz, uint32_t ArSz>
        requires TmplDefs::ATpIsdValidUniformWithArr<Tp, Rk, Sz, ArSz>
    struct AnthemUBDesc;

    template<typename... AUBTpDesc>
    class AnthemUniformBufferImpl;

    template<uint32_t Rk>
    using AnthemUniformMatf = AnthemUBDesc<float, 2, Rk, 1>;

    template<uint32_t Rk>
    using AnthemUniformVecf = AnthemUBDesc<float, 1, Rk, 1>;

    template<uint32_t Rk, uint32_t Ar>
    using AnthemUniformVecfArray = AnthemUBDesc<float, 1, Rk, Ar>;

    template<uint32_t Rk>
    using AtUniformVecf = AnthemUniformVecf<Rk>;

    template<uint32_t Rk>
    using AtUniformMatf = AnthemUniformMatf<Rk>;


    template<uint32_t Rk, uint32_t Ar>
    using AtUniformVecfArray = AnthemUniformVecfArray<Rk, Ar>;


    template<typename... UniTp, uint32_t... UniRk, uint32_t... UniSz, uint32_t... UniArrSz>
    class AnthemUniformBufferImpl<AnthemUBDesc<UniTp, UniRk, UniSz, UniArrSz>...> :public AnthemUniformBuffer {
    private:
        std::array<uint32_t, sizeof...(UniRk)> uniRanks = { UniRk... };
        std::array<uint32_t, sizeof...(UniSz)> uniVecSize = { UniSz... };
        std::array<uint32_t, sizeof...(UniArrSz)> uniArraySize = { UniArrSz... };
        std::array<uint32_t, sizeof...(UniTp)> uniTpSizeOf = { sizeof(UniTp)... };
        std::array<bool, sizeof...(UniTp)> uniIsFloat = { std::is_same<UniTp,float>::value ... };
        std::array<bool, sizeof...(UniTp)> uniIsInt = { std::is_same<UniTp,int>::value ... };

        std::vector<int> alignOffsets = {};
        std::vector<int> variableSize = {};

        char* rawBuffer = nullptr;

    protected:

        size_t getRequiredBufferSize() {
            size_t totalSize = 0;
            return alignOffsets.at(alignOffsets.size() - 1) + variableSize.at(variableSize.size() - 1);
        }
        bool allocateBuffer() {
            auto sz = getRequiredBufferSize();
            rawBuffer = new char[sz];
            memset(rawBuffer, 0, sz);
            return true;
        }
        bool calculatingOffset() {
            int curOffset = 0;
            for (uint32_t i = 0; i < sizeof...(UniTp); i++) {
                for (uint32_t j = 0; j < uniArraySize.at(i); j++) {
                    int seqLength = uniVecSize.at(i) * ((uniVecSize.at(i) - 1) * (uniRanks.at(i) == 2) + 1) * uniTpSizeOf.at(i);
                    int reqAlign = uniTpSizeOf.at(i);
                    if (uniVecSize.at(i) == 1) { //Scalar
                        reqAlign *= 1;
                    }
                    else if (uniVecSize.at(i) == 2 && uniRanks.at(i) == 1) { //Vec2
                        reqAlign *= 2;
                    }
                    else if (uniVecSize.at(i) == 3 && uniRanks.at(i) == 1) { //Vec3
                        reqAlign *= 4;
                    }
                    else if (uniVecSize.at(i) == 4 && uniRanks.at(i) == 1) { //Vec4
                        reqAlign *= 4;
                    }
                    else if (uniVecSize.at(i) == 4 && uniRanks.at(i) == 2) { //Mat4
                        reqAlign *= 4;
                    }
                    else {
                        ANTH_LOGE("Unsupported uniform type");
                        return false;
                    }
                    int alignedOffset = 0;
                    if (curOffset % reqAlign == 0) {
                        alignedOffset = curOffset;
                    }
                    else {
                        alignedOffset = (curOffset - curOffset % reqAlign) + reqAlign;
                    }
                    alignOffsets.push_back(alignedOffset);
                    variableSize.push_back(seqLength);
                    curOffset = alignedOffset + seqLength;
                }
            }
            return true;
        }
    protected:
        bool getRawBufferData(void** ptr) override {
            *ptr = rawBuffer;
            return true;
        }
        size_t getBufferSize() override {
            return this->getRequiredBufferSize();
        }
    public:
        AnthemUniformBufferImpl() {
            this->calculatingOffset();
            this->allocateBuffer();
        }
        ~AnthemUniformBufferImpl() {
            delete[] rawBuffer;
        }
        bool specifyUniforms(UniTp*... variables) {
            std::array<void*, sizeof...(UniTp)> voidPtrs = { variables... };
            int idx = 0;
            for (uint32_t i = 0; i < sizeof...(UniTp); i++) {
                for (uint32_t j = 0; j < uniArraySize.at(i); j++) {
                    int seqLength = uniVecSize.at(i) * ((uniVecSize.at(i) - 1) * (uniRanks.at(i) == 2) + 1) * uniTpSizeOf.at(i);
                    char* vdptr = ((char*)voidPtrs.at(i)) + seqLength * j;
                    memcpy(rawBuffer + alignOffsets.at(idx), vdptr, variableSize.at(idx));
                    idx += 1;
                }
            }
            return true;
        }
    };
}