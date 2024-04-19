#pragma once
#include "./AnthemPostprocessPass.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemBloom :virtual public AnthemPostprocessPass {
    private:
        uint32_t layers = 0;
        AnthemShaderFilePaths shaderDownPath;
        AnthemShaderModule* shaderDown;
        std::vector<AnthemFramebuffer*> downFramebuffers;
        std::vector<AnthemRenderPass*> downRenderPasses;
        std::vector<AnthemGraphicsPipeline*> downPipelines;
        std::vector<AnthemImage*> downColorAttachments;
        std::vector<AnthemDescriptorPool*> downDescriptorPools;
        std::vector<AnthemViewport*> downViewports;
        AnthemRenderPassSetupOption downRopt;
        AnthemGraphicsPipelineCreateProps downPopt;
        uint32_t width;
        uint32_t height;
        AnthemDescriptorPool* srcImg = nullptr;
        AnthemDescriptorPool* downChain = nullptr;
        std::vector<AnthemPushConstantImpl<AtBufVecFloat<1>>*> downPc;
    public:
        virtual void prepareShader() override;
        AnthemBloom(AnthemSimpleToyRenderer* p, uint32_t cmdCopies,uint32_t layers,uint32_t wid,uint32_t height);
        void prepareDownsample();
        void setSrcImage(AnthemDescriptorPool* src);
        virtual void prepare(bool offscreen) override;
        virtual void recordCommand() override;

    };
}