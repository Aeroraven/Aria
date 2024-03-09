#pragma once
#include "../../base/AnthemBaseImports.h"

namespace Anthem::Core{
    class AnthemBufferMemAligner{

    };

    // Type Descriptors
    template<typename Tp,uint32_t MatDim,uint32_t VecSz,uint32_t ArrSz>
    concept AnthemBufferVarDefConcept =  TmplDefs::ATpdAnyOf<Tp,int,unsigned int,float,double> && 
        TmplDefs::ATpdValueInRange<uint32_t,MatDim,1,2> &&
        TmplDefs::ATpdValueInRange<uint32_t,VecSz,1,4>;

    template<typename Tp,uint32_t MatDim,uint32_t VecSz,uint32_t ArrSz>
    struct AnthemBufferVarDefBase{
        constexpr static uint32_t matDim = MatDim, vecSize = VecSz, arrSize = ArrSz;
        constexpr static bool dynamicSize = false;
    };
    
    template<typename Tp, uint32_t AlignReq,uint32_t ElemLen>
    struct AnthemBufferVarDefDefinedMx{
        constexpr static bool typeDefined = true;
        constexpr static uint32_t alignReq = sizeof(Tp) * AlignReq;
        constexpr static uint32_t dataLength = sizeof(Tp) * ElemLen;
    };

    template<typename Tp,uint32_t MatDim,uint32_t VecSz,uint32_t ArrSz>
    requires AnthemBufferVarDefConcept<Tp,MatDim,VecSz,ArrSz>
    struct AnthemBufferVarDef:virtual AnthemBufferVarDefBase<Tp,MatDim,VecSz,ArrSz>{
        constexpr static bool typeDefined = false;
        constexpr static uint32_t alignReq = 1;
    };

    template<typename Tp,uint32_t ArrSz>
    struct AnthemBufferVarDef<Tp,1,1,ArrSz>:virtual AnthemBufferVarDefBase<Tp,1,1,ArrSz>,virtual AnthemBufferVarDefDefinedMx<Tp,1,1>{};
    
    template<typename Tp,uint32_t ArrSz>
    struct AnthemBufferVarDef<Tp,1,2,ArrSz>:virtual AnthemBufferVarDefBase<Tp,1,2,ArrSz>,virtual AnthemBufferVarDefDefinedMx<Tp,2,2>{};

    template<typename Tp,uint32_t ArrSz>
    struct AnthemBufferVarDef<Tp,1,3,ArrSz>:virtual AnthemBufferVarDefBase<Tp,1,3,ArrSz>,virtual AnthemBufferVarDefDefinedMx<Tp,4,3>{};

    template<typename Tp,uint32_t ArrSz>
    struct AnthemBufferVarDef<Tp,1,4,ArrSz>:virtual AnthemBufferVarDefBase<Tp,1,4,ArrSz>,virtual AnthemBufferVarDefDefinedMx<Tp,4,4>{};

    template<typename Tp,uint32_t ArrSz>
    struct AnthemBufferVarDef<Tp,2,4,ArrSz>:virtual AnthemBufferVarDefBase<Tp,2,4,ArrSz>,virtual AnthemBufferVarDefDefinedMx<Tp,4,16>{};

    template<typename Tp,uint32_t MatDim,uint32_t VecSz,uint32_t ArrSz>
    requires AnthemBufferVarDefConcept<Tp,MatDim,VecSz,ArrSz>
    struct AnthemBufferVarDynamicDef : AnthemBufferVarDef<Tp,MatDim,VecSz,ArrSz>{
        constexpr static bool dynamicSize = true;
    };

    template<uint32_t ArrSz= 1>
    using AtBufVecFloat = AnthemBufferVarDef<float,1,1,ArrSz>;

    template<uint32_t ArrSz= 1>
    using AtBufVec2f = AnthemBufferVarDef<float,1,2,ArrSz>;

    template<uint32_t ArrSz= 1>
    using AtBufVec3f = AnthemBufferVarDef<float,1,3,ArrSz>;

    template<uint32_t ArrSz= 1>
    using AtBufVec4f = AnthemBufferVarDef<float,1,4,ArrSz>;

    template<uint32_t ArrSz= 1>
    using AtBufMat4f = AnthemBufferVarDef<float,2,4,ArrSz>;

    template<uint32_t ArrSz = 1>
    using AtBufVecdFloat = AnthemBufferVarDynamicDef<float, 1, 1, ArrSz>;

    template<uint32_t ArrSz = 1>
    using AtBufVecd2f = AnthemBufferVarDynamicDef<float, 1, 2, ArrSz>;

    template<uint32_t ArrSz = 1>
    using AtBufVecd3f = AnthemBufferVarDynamicDef<float, 1, 3, ArrSz>;

    template<uint32_t ArrSz = 1>
    using AtBufVecd4f = AnthemBufferVarDynamicDef<float, 1, 4, ArrSz>;

    template<uint32_t ArrSz = 1>
    using AtBufMatd4f = AnthemBufferVarDynamicDef<float, 2, 4, ArrSz>;

    template<typename... Desc>
    class AnthemBufferMemAlignerImpl;

    template< template <typename Tp,uint32_t MatDim,uint32_t VecSz,uint32_t ArrSz> class... DescTp, 
        typename... Tp,uint32_t... MatDim,uint32_t... VecSz,uint32_t... ArrSz>
    requires ((std::is_base_of_v<DescTp<Tp,MatDim,VecSz,ArrSz>,AnthemBufferVarDef<Tp,MatDim,VecSz,ArrSz>> || 
        std::is_base_of_v<DescTp<Tp,MatDim,VecSz,ArrSz>,AnthemBufferVarDynamicDef<Tp,MatDim,VecSz,ArrSz>> ) && ...)
    class AnthemBufferMemAlignerImpl<DescTp<Tp,MatDim,VecSz,ArrSz>...>:public virtual AnthemBufferMemAligner{
    
    public:
        using IdxType = uint32_t;
        constexpr static IdxType numArgs = sizeof...(DescTp);
        constexpr static IdxType totlArrayElements = ((ArrSz) + ...);
        constexpr static std::array<bool,numArgs> dynamicSize = {(DescTp<Tp,MatDim,VecSz,ArrSz>::dynamicSize)...};
        constexpr static std::array<bool,numArgs> validType = {(DescTp<Tp,MatDim,VecSz,ArrSz>::typeDefined)...};
        constexpr static std::array<IdxType,numArgs> alignRequirement = {(DescTp<Tp,MatDim,VecSz,ArrSz>::alignReq)...};
        constexpr static std::array<IdxType,numArgs> elemSize = {(DescTp<Tp,MatDim,VecSz,ArrSz>::dataLength)...};
        constexpr static std::array<IdxType,numArgs> arrayElements = {ArrSz...};


        constexpr static std::array<bool, numArgs> isFloatType = { std::is_floating_point<Tp>::value... };
        constexpr static std::array<bool, numArgs> isIntType = { std::is_integral<Tp>::value... };
        constexpr static std::array<bool, numArgs> isUnsignedType = { std::is_unsigned<Tp>::value... };
        constexpr static std::array<IdxType, numArgs> attrTpSize = { sizeof(Tp)... };
        constexpr static std::array<IdxType, numArgs> attrDims = { VecSz... };

        constexpr static IdxType getDynamicVarStartPos(){
            for(auto i=0;i<numArgs;i++){
                if(dynamicSize.at(i)){
                    return i;
                }
            }
            return numArgs;
        }
        constexpr static IdxType dynVarStart = getDynamicVarStartPos();
        constexpr static bool legalityAssertion(){
            int numVertexStageVars = 0;
            if constexpr(numArgs==0) return false;
            for (auto i = 0; i < numArgs; i++) {
                if (!validType.at(i)) return false;
            }
            for (auto i = dynVarStart; i < numArgs; i++) {
                if (!dynamicSize.at(i) || arrayElements.at(i) != 1) return false;
            }
            return true;
        }
        static_assert(legalityAssertion());

        constexpr static IdxType getUniformElementCounts(){
            int r = 0;
            for(int i=0;i<dynVarStart;i++) r+=arrayElements.at(i);
            return r;
        }
        constexpr static IdxType uniformElemCnt = getUniformElementCounts();
        constexpr static IdxType dynamicElemCnt = totlArrayElements - uniformElemCnt;
        static_assert((uniformElemCnt==0||dynamicElemCnt==0));

    private:
        char* buffer = nullptr;
    
    public:
        std::array<IdxType,uniformElemCnt> bmaGetUniformAlignOffsets(){
            std::array<IdxType,uniformElemCnt> alignReqs;
            int cIdx = 0;
            int curOffset = 0;
            for(IdxType i=0;i<dynVarStart;i++){
                for(IdxType j=0;j<arrayElements.at(i);j++){
                    auto diff = curOffset%alignRequirement.at(i);
                    if(diff!=0) curOffset += alignRequirement.at(i)-diff;
                    alignReqs[cIdx++] = static_cast<IdxType>(curOffset);
                    curOffset+=elemSize.at(i);
                }
            } 
            return alignReqs;
        }
        uint32_t bmaGetUniformAlignOffsetsAll() {
            std::array<IdxType, uniformElemCnt> alignReqs;
            int cIdx = 0;
            int curOffset = 0;
            for (IdxType i = 0; i < dynVarStart; i++) {
                for (IdxType j = 0; j < arrayElements.at(i); j++) {
                    auto diff = curOffset % alignRequirement.at(i);
                    if (diff != 0) curOffset += alignRequirement.at(i) - diff;
                    alignReqs[cIdx++] = static_cast<IdxType>(curOffset);
                    curOffset += elemSize.at(i);
                }
            }
            return curOffset;
        }
        std::array<IdxType,uniformElemCnt> uniformOffsetReq = bmaGetUniformAlignOffsets();

        std::array<IdxType,dynamicElemCnt> bmaGetDynamicAlignOffsets(){
            std::array<IdxType,dynamicElemCnt> alignReqs;
            int curOffset = 0;
            int cIdx = 0;
            for(int i=dynVarStart;i<numArgs;i++){
                for(int j=0;j<arrayElements.at(i);j++){
                    auto diff = curOffset%alignRequirement.at(i);
                    if(diff!=0) curOffset += alignRequirement.at(i)-diff;
                    alignReqs[cIdx++] = curOffset;
                    curOffset+=elemSize.at(i);
                }
            }
            return alignReqs;
        }
        std::array<IdxType,dynamicElemCnt> dynamicOffsetReq = bmaGetDynamicAlignOffsets();

        IdxType bmaGetPaddingBeforeNextDynElement(){
            if constexpr(dynamicElemCnt == 0){
                return 0;
            }
            auto alignReqFirst = alignRequirement.at(dynVarStart);
            auto lastOffset = dynamicOffsetReq.back()+elemSize.back();
            if(lastOffset%alignReqFirst!=0){
                return alignReqFirst-lastOffset%alignReqFirst;
            } 
            return 0;
        }

        IdxType bmaGetIthElementPrePadding(IdxType idx){
            auto lastOffset = dynamicOffsetReq.back()+elemSize.back();
            auto padding = bmaGetPaddingBeforeNextDynElement();
            auto totlPad = lastOffset+padding;
            //ANTH_LOGI("Last off:", lastOffset, " Padding:", padding," Idx:",idx);
            return idx*totlPad;
        }

    public:
        bool bmaBindBuffer(char* ptr){
            this->buffer = ptr;
            return true;
        }
        bool bmaSetDynamicInput(IdxType idx, std::array<Tp,VecSz>... args){
            std::vector<char*> ptrs = {(reinterpret_cast<char*>(args.data()))...};
            auto basePads = bmaGetIthElementPrePadding(idx);
            for(int i=0;i<numArgs;i++){
                auto idxOffset = dynamicOffsetReq.at(i);
                memcpy(buffer+basePads+idxOffset,ptrs.at(i),elemSize.at(i));
            }
            return true;
        }
        bool bmaSetAllUniformInput(std::array<Tp, VecSz>... args) {
            std::vector<char*> ptrs = { (reinterpret_cast<char*>(args.data()))... };
            for (int i = 0; i < numArgs; i++) {
                auto idxOffset = uniformOffsetReq.at(i);
                memcpy(buffer + idxOffset, ptrs.at(i), elemSize.at(i));
            }
            return true;
        }
        bool bmaSetAllUniformInput2(Tp*... args) {
            std::vector<char*> ptrs = { reinterpret_cast<char*>(args)... };
            for (int i = 0; i < numArgs; i++) {
                auto idxOffset = uniformOffsetReq.at(i);
                memcpy(buffer + idxOffset, ptrs.at(i), elemSize.at(i));
            }
            return true;
        }

        IdxType bmaCalcRequiredSpaceForDynInput(IdxType numElements){
            return bmaGetIthElementPrePadding(numElements+1);
        }
    };
}