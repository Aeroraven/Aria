#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemPostIdentity :virtual public AnthemPostprocessPass {
    public:
        virtual void prepareShader() override;
        AnthemPostIdentity(AnthemSimpleToyRenderer* p, uint32_t cmdCopies);
    };
}