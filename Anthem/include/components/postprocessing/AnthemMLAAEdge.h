#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemMLAAEdge :virtual public AnthemPostprocessPass {
    protected:
        uint32_t searchRange;
    public:
        virtual void prepareShader() override;
        AnthemMLAAEdge(AnthemSimpleToyRenderer* p, uint32_t cmdCopies, uint32_t searchRanges);
    };
}