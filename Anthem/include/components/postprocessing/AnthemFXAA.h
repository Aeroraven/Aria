#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemFXAA:virtual public AnthemPostprocessPass {
    public:
        virtual void prepareShader() override;
        AnthemFXAA(AnthemSimpleToyRenderer* p, uint32_t cmdCopies);
    };
}