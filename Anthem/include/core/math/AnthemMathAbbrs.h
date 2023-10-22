#pragma once

#include "./AnthemLinAlg.h"
namespace Anthem::Core::Math::Abbr{
    using AtMatf4 = ALinAlgMat<float,4,4>;
    using AtMatf3 = ALinAlgMat<float,3,3>;
    using AtMatf2 = ALinAlgMat<float,2,2>;
    using AtVecf4 = ALinAlgVec<float,4>;
    using AtVecf3 = ALinAlgVec<float,3>;
    using AtVecf2 = ALinAlgVec<float,2>;

    using AtMatd4 = ALinAlgMat<double,4,4>;
    using AtMatd3 = ALinAlgMat<double,3,3>;
    using AtMatd2 = ALinAlgMat<double,2,2>;
    using AtVecd4 = ALinAlgVec<double,4>;
    using AtVecd3 = ALinAlgVec<double,3>;
    using AtVecd2 = ALinAlgVec<double,2>;

    using AtMati4 = ALinAlgMat<int,4,4>;
    using AtMati3 = ALinAlgMat<int,3,3>;
    using AtMati2 = ALinAlgMat<int,2,2>;
    using AtVeci4 = ALinAlgVec<int,4>;
    using AtVeci3 = ALinAlgVec<int,3>;
    using AtVeci2 = ALinAlgVec<int,2>;
}