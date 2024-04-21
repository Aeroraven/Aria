#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemSimpleBlur :virtual public AnthemPostprocessPass {
    public:
        virtual void prepareShader() override;
        AnthemSimpleBlur(AnthemSimpleToyRenderer* p, uint32_t cmdCopies);
    };
}