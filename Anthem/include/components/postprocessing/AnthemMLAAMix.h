#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemMLAAMix :virtual public AnthemPostprocessPass {
    protected:
        uint32_t searchRange;
    public:
        virtual void prepareShader() override;
        AnthemMLAAMix(AnthemSimpleToyRenderer* p, uint32_t cmdCopies, uint32_t searchRanges);
    };
}