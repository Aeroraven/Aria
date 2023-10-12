#include "include/entry/AnthemEnvImpl.h"

int main(){
    ANTH_LOGI("Hello World!");
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto app = ANTH_MAKE_SHARED(Anthem::Entry::AnthemEnvImpl)(cfg);
    app->run();
    return 0;
}
