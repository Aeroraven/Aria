#include "include/entry/AnthemEnvImpl.h"
using namespace Anthem::Core;
using namespace Anthem::Entry;

int main(){
    std::regex r("(<.*>|\\[.*\\])");
    std::string w = "Anthem::Core::AnthemVertexBufferImpl<Anthem::Core::AnthemVAOAttrDesc<AttrTp, AttrSz>...>::AnthemVertexBufferImpl() [with AttrTp = {float, float}; unsigned int ...AttrSz = {2, 3}]";
    ANTH_LOGI(std::regex_replace(w,r,""));
    auto g = std::regex_replace(w,r,"");
    auto p = g.find(' ');
    ANTH_LOGI("P=",p,",",g.size());
    ANTH_LOGI("Goodbye World!");
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto app = ANTH_MAKE_SHARED(Anthem::Entry::AnthemEnvImpl)(cfg);
    app->run();
    return 0;
}
