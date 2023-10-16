#pragma once
#include "AnthemMatrix.h"

namespace Anthem::Core::Math{
    template<typename T,uint32_t R,uint32_t C>
    using ALinAlgMat = AnthemMatrix<T,R,C>;

    class AnthemLinAlg{
    public:
        template<typename T,uint32_t R>
        static ALinAlgMat<T,R,R> eye(){
            ALinAlgMat<T,R,R> out;
            for (uint32_t i = 0; i < R; i++){
                for (uint32_t j = 0; j < R; j++){
                    out[i][j] = (i==j)?1:0;
                }
            }
            return out;
        }
    };
}