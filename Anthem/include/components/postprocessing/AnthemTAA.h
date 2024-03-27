#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemTAA :virtual public AnthemPostprocessPass {
    public:
        virtual void prepareShader() override;
        AnthemTAA(AnthemSimpleToyRenderer* p, uint32_t cmdCopies);
    };
}