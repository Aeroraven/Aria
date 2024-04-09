#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemGlobalFog :virtual public AnthemPostprocessPass {
    public:
        virtual void prepareShader() override;
        AnthemGlobalFog(AnthemSimpleToyRenderer* p, uint32_t cmdCopies);
    };
}