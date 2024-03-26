#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemMLAABlend :virtual public AnthemPostprocessPass {
    protected:
        uint32_t searchRange;
    public:
        virtual void prepareShader() override;
        virtual void prepareBlendTexture(std::string path);
        AnthemMLAABlend(AnthemSimpleToyRenderer* p, uint32_t cmdCopies, uint32_t searchRanges);
    };
}