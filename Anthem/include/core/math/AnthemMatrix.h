#pragma once
#include "../base/AnthemBaseImports.h"

namespace Anthem::Core::Math{
    
    template<typename T,uint32_t R,uint32_t C>
    concept AnthemMatrixLegalConcept = TmplDefs::ATpdAnyOf<T,float,int,double,unsigned int> && (R>0) && (C>0);

    template<typename T,uint32_t R,uint32_t C>
    requires AnthemMatrixLegalConcept<T,R,C>
    class AnthemMatrix{
    using AnthemSameShapeMatrixTp = AnthemMatrix<T,R,C>;
    private:
        T data[R][C];

    public:
    T* operator[](uint32_t i){
        return data[i];
    }
    const T* operator[](uint32_t i) const{
        return data[i];
    }
    public:
        AnthemMatrix(){
            for(int i=0;i<R;i++){
                for(int j=0;j<C;j++){
                    data[i][j] = 0;
                }
            }
        }
        const AnthemMatrix<T,R,C>& operator=(const AnthemMatrix<T,R,C>& other){
            for(int i=0;i<R;i++){
                for(int j=0;j<C;j++){
                    data[i][j] = other.data[i][j];
                }
            }
            return *this;
        }
        void dot(const AnthemMatrix<T,R,C>& other,AnthemMatrix<T,R,C>& out){
            for (uint32_t i = 0; i < R; i++){
                for (uint32_t j = 0; j < C; j++){
                    out.data[i][j] = data[i][j] * other.data[i][j];
                }
            }
        }

        template<typename T2, uint32_t R2, uint32_t C2>
        requires (R2 == C)
        AnthemMatrix<T2,R,C2> multiply(const AnthemMatrix<T2,R2,C2>& other){
            AnthemMatrix<T2,R,C2> out;
            for (uint32_t i = 0; i < R; i++){
                for (uint32_t j = 0; j < C2; j++){
                    T2 sum = 0;
                    for (uint32_t k = 0; k < C; k++){
                        sum += data[i][k] * other[k][j];
                    }
                    out[i][j] = sum;
                }
            }
            return out;
        }
        void print() const{
            for (uint32_t i = 0; i < R; i++){
                for (uint32_t j = 0; j < C; j++){
                    std::cout << data[i][j] << " ";
                }
                std::cout << std::endl;
            }
        }
        void rowMajorVectorization(T* out) const{
            for (uint32_t i = 0; i < R; i++){
                for (uint32_t j = 0; j < C; j++){
                    out[i*C+j] = data[i][j];
                }
            }
        }
        void columnMajorVectorization(T* out) const{
            for (uint32_t i = 0; i < C; i++){
                for (uint32_t j = 0; j < R; j++){
                    out[i*R+j] = data[j][i];
                }
            }
        }

    };
}