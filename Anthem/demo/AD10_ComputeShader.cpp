#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::External;
using namespace Anthem::Components::Camera;


int main(){
    ANTH_LOGI("Meow!");
    AnthemBufferMemAlignerImpl<
        AnthemBufferVarDynamicDef<float,1,1,1>,
        AnthemBufferVarDynamicDef<float,1,3,1>,
        AnthemBufferVarDynamicDef<float,1,2,1>,
        AnthemBufferVarDynamicDef<float,2,4,1>,
        AnthemBufferVarDynamicDef<float,1,1,1>,
        AnthemBufferVarDynamicDef<float,1,1,1>,
        AnthemBufferVarDynamicDef<float,1,1,1>
    > a;
    auto x = a.dynamicOffsetReq;
    for(auto w:x){
        ANTH_LOGI(w);
    }
    return 0;
}