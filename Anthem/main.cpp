#include "include/entry/AnthemEnvImpl.h"
using namespace Anthem::Core;
using namespace Anthem::Entry;

int main(){
    ANTH_LOGI("Goodbye World!");
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto app = ANTH_MAKE_SHARED(Anthem::Entry::AnthemEnvImpl)(cfg);
    app->run();
    return 0;
}
