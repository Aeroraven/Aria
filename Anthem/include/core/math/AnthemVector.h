#pragma once
#include "../base/AnthemBaseImports.h"

namespace Anthem::Core::Math{
    template<typename T,uint32_t R>
    concept AnthemVectorLegalConcept = TmplDefs::ATpdAnyOf<T,float,int,double,unsigned int> && (R>0);

    template<typename T,uint32_t R>
    requires AnthemVectorLegalConcept<T,R>
    class AnthemVector{
    using AnthemSameShapeVectorTp = AnthemVector<T,R>;
    private:
        T data[R];

    public:
        template<typename... U>
        requires TmplDefs::ATpdAllAre<T,U...>
        AnthemVector(U... args){
            static_assert(sizeof...(args)==R);
            uint32_t i=0;
            ((data[i++]=args),...);
        }
        AnthemVector(){
            for(int i=0;i<R;i++){
                data[i] = 0;
            }
        }
        T& operator[](uint32_t i){
            return data[i];
        }
        T operator[](uint32_t i) const{
            return data[i];
        }
        AnthemSameShapeVectorTp operator-(const AnthemSameShapeVectorTp& second) const{
            AnthemSameShapeVectorTp r;
            for(int i=0;i<R;i++){
                r[i]=data[i]-second.data[i];
            }
            return r;
        }
        T len() const{
            T r = 0;
            for(int i=0;i<R;i++){
                r += data[i]*data[i];
            }
            return static_cast<T>(sqrt(r));
        }
        void print() const{
            std::cout << "[";
            for(int i=0;i<R;i++){
                std::cout << data[i] << ((i==R-1)?"":",");
            }
            std::cout << "]" << std::endl;
        }
        AnthemSameShapeVectorTp& normalize_(){
            auto l = len();
            for(int i=0;i<R;i++){
                data[i] /= l;
            }
            return *this;
        }
        AnthemSameShapeVectorTp normalize() const{
            AnthemSameShapeVectorTp out;
            auto l = len();
            for(int i=0;i<R;i++){
                out[i] = data[i] / l;
            }
            return out;
        }
    };
}