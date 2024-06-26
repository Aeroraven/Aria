#pragma once
#include "AnthemMatrix.h"
#include "AnthemVector.h"
#include "AnthemQuaternion.h"

namespace Anthem::Core::Math{

    template<typename T,uint32_t R,uint32_t C>
    using ALinAlgMat = AnthemMatrix<T,R,C>;

    template<typename T,uint32_t R>
    using ALinAlgVec = AnthemVector<T,R>;

    template<typename T>
    concept ALinAlgIsNumericTp = std::is_arithmetic_v<T>;

    class AnthemLinAlg{
    using This=AnthemLinAlg;
    
    private:
        template<typename T> 
        requires ALinAlgIsNumericTp<T>
        inline static T atUniformNumber(T s,T t){
            std::random_device rd;
            std::mt19937 generator(rd());
            std::uniform_real_distribution<float> unf(s,t);
            auto res = unf(generator);
            return res;
        }
    public:
        template<typename T,uint32_t R>
        inline static ALinAlgMat<T,R,R> eye(){
            ALinAlgMat<T,R,R> out;
            for (uint32_t i = 0; i < R; i++){
                for (uint32_t j = 0; j < R; j++){
                    out[i][j] = (i==j)?1:0;
                }
            }
            return out;
        }
    
        template<typename T,uint32_t R>
        inline static ALinAlgMat<T,R,R> identity(){
            return This::eye<T,R>();
        }

        template<typename T,uint32_t R, uint32_t C>
        inline static ALinAlgVec<T,R> linearTransform(const ALinAlgMat<T,R,C>& t,const ALinAlgVec<T,C>& v){
            ALinAlgVec<T,R> out;
            for (uint32_t i = 0; i < R; i++){
                T sum = 0;
                for (uint32_t j = 0; j < C; j++){
                    sum += t[i][j] * v[j];
                }
                out[i] = sum;
            }
            return out;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T,4,4> spatialOrthoTransform(T zNear, T zFar, T nLeft, T nRight, T nTop, T nBottom){
            ALinAlgMat<T,4,4> out;
            // x'=Ax+b => -1=AL+b 1=AR+b =>A(L+R)+2b=0 
            // 2=A(R-L) => A=2/(R-L) => 2b+2/(R-L)*(R+L) = 0 
            // b=-(R+L)/(R-L) A=2/(R-L)

            //For Depth -> 0-1
            //0=AL+b 1=AR+b =>b=-AL=>1=AR-AL=A(R-L)=>A=1/(R-L) => b=-AL = -L/(R-L)
            out[0][0] = 2/(nRight-nLeft);
            out[1][1] = 2/(nTop-nBottom);
            out[2][2] = 1/(zFar-zNear);
            out[3][3] = 1;
            out[0][3] = -(nRight+nLeft)/(nRight-nLeft);
            out[1][3] = -(nTop+nBottom)/(nTop-nBottom);
            out[2][3] = -(zNear)/(zFar-zNear);
            return out;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T,4,4> spatialPerspectiveTransform(T zNear, T zFar, T nLeft, T nRight, T nTop, T nBottom){
            ALinAlgMat<T,4,4> out;
            out[0][0] = 2*zNear/(nRight-nLeft);
            out[1][1] = 2*zNear/(nTop-nBottom);
            out[2][2] = (zFar+zNear)/(zFar-zNear);
            out[3][2] = 1;
            out[2][3] = -2*zFar*zNear/(zFar-zNear);
            out[3][3] = 0;
            out[0][2] = -(nRight+nLeft)/(nRight-nLeft);
            out[1][2] = -(nTop+nBottom)/(nTop-nBottom);
            return out;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T,4,4> spatialPerspectiveTransformWithFovAspect(T zNear, T zFar, T nFovy,T aspect){
            ALinAlgMat<T,4,4> out;
            T halfFovy = nFovy/static_cast<T>(2.0);
            T nTop = tan(halfFovy) * zNear;
            T nBottom = -nTop;
            T nLeft = -aspect*nTop;
            T nRight = -nLeft;
            out[0][0] = 2*zNear/(nRight-nLeft);
            out[1][1] = 2*zNear/(nTop-nBottom);
            //out[2][2] = (zFar+zNear)/(zFar-zNear);
            out[2][2] = (zFar)/(zFar-zNear);
            out[3][2] = 1;
            //out[2][3] = -2*zFar*zNear/(zFar-zNear);
            out[2][3] = -1*zFar*zNear/(zFar-zNear);
            out[3][3] = 0;
            out[0][2] = -(nRight+nLeft)/(nRight-nLeft);
            out[1][2] = -(nTop+nBottom)/(nTop-nBottom);
            return out;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T,4,4> spatialPerspectiveTransformWithFovAspectNegateY(T zNear, T zFar, T nFovy,T aspect){
            ALinAlgMat<T,4,4> out;
            T halfFovy = nFovy/static_cast<T>(2.0);
            T nTop = tan(halfFovy) * zNear;
            T nBottom = -nTop;
            T nLeft = -aspect*nTop;
            T nRight = -nLeft;
            out[0][0] = 2*zNear/(nRight-nLeft);
            out[1][1] = -2*zNear/(nTop-nBottom);
            out[2][2] = (zFar)/(zFar-zNear);
            out[3][2] = 1;
            out[2][3] = -1*zFar*zNear/(zFar-zNear);
            out[3][3] = 0;
            out[0][2] = -(nRight+nLeft)/(nRight-nLeft);
            out[1][2] = (nTop+nBottom)/(nTop-nBottom);
            return out;
        }



        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgVec<T,3> cross3(const ALinAlgVec<T,3>& a, const ALinAlgVec<T,3>& b){
            ALinAlgVec<T,3> ret;
            ret[0] = a[1]*b[2]-a[2]*b[1];
            ret[1] = a[2]*b[0]-a[0]*b[2];
            ret[2] = a[0]*b[1]-a[1]*b[0];
            return ret;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T,4,4> lookAtTransform(const ALinAlgVec<T,3>& e,const ALinAlgVec<T,3>& c,const ALinAlgVec<T,3>& u){
            auto z = (c-e).normalize_();
            auto un = u.normalize();
            auto x = This::cross3(un,z);
            auto y = This::cross3(z,x);
            ALinAlgMat<T,4,4> rot;
            ALinAlgMat<T,4,4> trans;
            for(int i=0;i<3;i++){
                rot[0][i] = x[i];
                rot[1][i] = y[i];
                rot[2][i] = z[i];
                trans[i][3] = -e[i];
                trans[i][i] = 1;
            }
            rot[3][3] = 1;
            trans[3][3] = 1;
            auto ret = rot.multiply(trans);
            return ret;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T,3,3> crossProductAsTransform(const ALinAlgVec<T,3>& a){
            ALinAlgMat<T,3,3> ret;
            ret[0][1] = -a[2];
            ret[0][2] = a[1];
            ret[1][2] = -a[0];
            ret[1][0] = a[2];
            ret[2][0] = -a[1];
            ret[2][1] = a[0];
            return ret;
        }

        template<typename T, typename U>
        requires ALinAlgIsNumericTp<T> && ALinAlgIsNumericTp<U>
        inline static ALinAlgMat<T,4,4> axisAngleRotationTransform3(const ALinAlgVec<T,3>& axis, U rad){
            auto axis_n = axis.normalize();
            auto c = static_cast<T>(std::cos(rad));
            auto s = static_cast<T>(std::sin(rad));
            auto t = 1-c;
            ALinAlgMat<T,4,4> ret;
            ret[0][0] = t*axis_n[0]*axis_n[0]+c;
            ret[0][1] = t*axis_n[0]*axis_n[1]-s*axis_n[2];
            ret[0][2] = t*axis_n[0]*axis_n[2]+s*axis_n[1];
            ret[1][0] = t*axis_n[0]*axis_n[1]+s*axis_n[2];
            ret[1][1] = t*axis_n[1]*axis_n[1]+c;
            ret[1][2] = t*axis_n[1]*axis_n[2]-s*axis_n[0];
            ret[2][0] = t*axis_n[0]*axis_n[2]-s*axis_n[1];
            ret[2][1] = t*axis_n[1]*axis_n[2]+s*axis_n[0];
            ret[2][2] = t*axis_n[2]*axis_n[2]+c;
            ret[3][3] = 1;
            return ret;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T,4,4> translationTransform3(const ALinAlgVec<T,3>& t){
            ALinAlgMat<T,4,4> ret;
            ret[0][0] = 1;
            ret[1][1] = 1;
            ret[2][2] = 1;
            ret[3][3] = 1;
            ret[0][3] = t[0];
            ret[1][3] = t[1];
            ret[2][3] = t[2];
            return ret;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T,3,3> inverse3(const ALinAlgMat<T,3,3>& m){
            //Adapted from:  https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
            T det = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
                        m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                        m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
            T invdet = 1 / det;
            ALinAlgMat<T,3,3> minv; 
            minv[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invdet;
            minv[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invdet;
            minv[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invdet;
            minv[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invdet;
            minv[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invdet;
            minv[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invdet;
            minv[2][0] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invdet;
            minv[2][1] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invdet;
            minv[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invdet;
            return minv;
        }

        template<typename T,uint32_t R>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgMat<T, R, R> gaussJordan(const ALinAlgMat<T, R, R>& a) {
            //Gauss
            auto ret = identity<T, R>();
            auto src = a;
            for (auto i : AT_RANGE2(R)) {
                auto factor = src[i][i];
                for (auto j : AT_RANGE(i + 1, R)) {
                    auto redFactor = src[j][i] / factor;
                    for (auto k : AT_RANGE2(R)) {
                        src[j][k] -= src[i][k] * redFactor;
                        ret[j][k] -= ret[i][k] * redFactor;
                    }
                }
            }
            //Jordan
            for (int i = R - 1; i >= 0; i--) {
                auto factor = src[i][i];
                for (auto j : AT_RANGE2(R)) {
                    src[i][j] /= factor;
                    ret[i][j] /= factor;
                }
                for (auto j : AT_RANGE2(i)) {
                    auto cf = src[j][i];
                    for (auto k : AT_RANGE2(R)) {
                        src[j][k] -= src[i][k] * cf;
                        ret[j][k] -= ret[i][k] * cf;
                    }
                }
            }
            return ret;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgVec<T,3> randomVector3(){
            const auto a = atUniformNumber(0.0f,2.0f*static_cast<float>(AT_PI));
            const auto b = atUniformNumber(0.0f,2.0f*static_cast<float>(AT_PI));
            const auto x = std::cos(a)*std::cos(b);
            const auto z = std::cos(a)*std::sin(b);
            const auto y = std::sin(b);
            ALinAlgVec<T,3> ret = {x,y,z};
            return ret;
        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static ALinAlgVec<T,3> randomVectorIid3(){
            const auto a = atUniformNumber(-1.0f,1.0f);
            const auto b = atUniformNumber(-1.0f,1.0f);
            const auto c = atUniformNumber(-1.0f,1.0f);
            ALinAlgVec<T,3> ret = {a,b,c};
            return ret;
            
        }

        template<typename T>
            requires ALinAlgIsNumericTp<T>
        inline static ALinAlgVec<T, 4> randomRgba() {
            const auto a = atUniformNumber(0.0f, 1.0f);
            const auto b = atUniformNumber(0.0f, 1.0f);
            const auto c = atUniformNumber(0.0f, 1.0f);
            const auto d = atUniformNumber(0.0f, 1.0f);
            ALinAlgVec<T, 4> ret = { a,b,c,d };
            return ret;

        }

        template<typename T>
        requires ALinAlgIsNumericTp<T>
        inline static T randomNumber(){
            return atUniformNumber(0.0f,1.0f);
        }
    };
}
/*
mat3 (float x1,float x2,float x3){
    x1 = x1*PI2;
    x2 = x2*PI2;
    mat3 R = mat3(cos(x1),sin(x1),0.0,-sin(x1),cos(x1),0.0,0.0,0.0,1.0);
    vec3 v = vec3(cos(x2)*sqrt(x3),sin(x2)*sqrt(x3),sqrt(1.0-x3));
    mat3 T = mat3(v.x*v.x,v.x*v.y,v.x*v.z,v.x*v.y,v.y*v.y,v.y*v.z,v.x*v.z,v.y*v.z,v.z*v.z);
    mat3 I = mat3(1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0);
    mat3 H = I - 2.0*T;
    return -H*R;
}*/