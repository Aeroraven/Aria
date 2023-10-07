#pragma once
#define ALW_WASM

#ifdef ALW_WASM
    #include <emscripten.h>
    #include <emscripten/bind.h>
    using namespace emscripten;

    #define ALW_ENABLE_EMCC_BINDINGS
    #define ALW_ENABLE_WASM

#else

#endif

#include <iostream>
#include <memory>
#include <cmath>
