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

    protected:
        void deepCopy(const AnthemMatrix<T,R,C>& other){
            for(int i=0;i<R;i++){
                for(int j=0;j<C;j++){
                    data[i][j] = other.data[i][j];
                }
            }
        }
        void deepCopyRvalue(AnthemMatrix<T,R,C>&& other){
            for(int i=0;i<R;i++){
                for(int j=0;j<C;j++){
                    data[i][j] = other.data[i][j];
                }
            }
        }
    public:
        T* operator[](uint32_t i){
            return data[i];
        }
        const T* operator[](uint32_t i) const{
            return data[i];
        }
        AnthemMatrix(){
            for(int i=0;i<R;i++){
                for(int j=0;j<C;j++){
                    data[i][j] = 0;
                }
            }
        }
        AnthemMatrix(const AnthemMatrix<T,R,C>& other){
            this->deepCopy(other);
        }
        AnthemMatrix(AnthemMatrix<T,R,C>&& other){
            this->deepCopy(other);
        }
        const AnthemMatrix<T,R,C>& operator=(const AnthemMatrix<T,R,C>& other){
            this->deepCopy(other);
            return *this;
        }
        const AnthemMatrix<T,R,C>& operator=(AnthemMatrix<T,R,C>&& other){
            this->deepCopy(other);
            return *this;
        }
        void dot(const AnthemMatrix<T,R,C>& other,AnthemMatrix<T,R,C>& out) const{
            for (uint32_t i = 0; i < R; i++){
                for (uint32_t j = 0; j < C; j++){
                    out.data[i][j] = data[i][j] * other.data[i][j];
                }
            }
        }
        template<typename T2, uint32_t R2, uint32_t C2>
        requires (R2 == C)
        AnthemMatrix<T2,R,C2> multiply(const AnthemMatrix<T2,R2,C2>& other) const{
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
        AnthemMatrix<T,C,R> transpose() const{
            AnthemMatrix<T,C,R> out;
            for(int i=0;i<R;i++){
                for(int j=0;j<C;j++){
                    out[j][i] = data[i][j];
                }
            }
            return out;
        }

        template<uint32_t Rp,uint32_t Cp>
        requires (Rp>=0) && (Cp>=0)
        AnthemMatrix<T,R+Rp,C+Cp> padRightBottom() const{
            AnthemMatrix<T,R+Rp,C+Cp> out;
            for(int i=0;i<R;i++){
                for(int j=0;j<C;j++){
                    out[i][j] = data[i][j];
                }
            }
            return out;
        }

        template<uint32_t Rp, uint32_t Cp>
        requires (Rp>=1) && (Cp>=1) && (Rp<=R) && (Cp<=C)
        AnthemMatrix<T,Rp,Cp> clipSubmatrixLeftTop() const{
            AnthemMatrix<T,Rp,Cp> out;
            for(int i=0;i<Rp;i++){
                for(int j=0;j<Cp;j++){
                    out[i][j] = data[i][j];
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