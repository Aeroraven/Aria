#include <iostream>
#include <memory>
#include "include/core/AnthemEnvImpl.h"

int main(){
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto x = ANTH_MAKE_SHARED(Anthem::Core::AnthemEnvImpl)(cfg);
    x->run();
    std::cout << "Hello World!" << std::endl;
    std::cout << std::boolalpha << (std::string("a")==std::string("a"));
    return 0;
}
