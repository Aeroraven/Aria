#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/renderer/AnthemSimpleToyRenderer.h"

namespace Anthem::Components::Postprocessing {
    using namespace Anthem::Core;
    class AnthemPostprocessPass {
    protected:
        AnthemShaderFilePaths shaderPath;
        AnthemShaderModule* shader;
        AnthemDepthBuffer* depthStencil;
        AnthemRenderPass* pass;
        AnthemGraphicsPipeline* pipeline;

        AnthemRenderPassSetupOption ropt;
        AnthemGraphicsPipelineCreateProps copt;

        AnthemFramebuffer* fbTarget;
        AnthemSwapchainFramebuffer* fbSwapchain;

        AnthemVertexBufferImpl<AtAttributeVecf<4>>* vx;
        AnthemIndexBuffer* ix;

        std::vector<std::vector<AnthemDescriptorSetEntry>> inputs;

        AnthemSimpleToyRenderer* rd;
        uint32_t cmdCopies = 2;
        std::vector<uint32_t> cmdIdx;

        std::vector<AnthemSemaphore*> procComplete;
    public:
        AnthemPostprocessPass(AnthemSimpleToyRenderer* p, uint32_t cmdCopies);

        virtual void prepareShader() = 0;
        virtual void prepareRenderPass();
        virtual void prepareGeometry();
        virtual void prepare();
        virtual void addInput(std::vector<AnthemDescriptorSetEntry> ins,int target=-1);

        virtual void recordCommand();
         
        uint32_t getCommandIdx(uint32_t id) const;

    };
}