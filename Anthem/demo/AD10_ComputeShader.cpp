#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"
#include "../include/core/drawing/buffer/AnthemBufferMemAligner.h"

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::External;
using namespace Anthem::Components::Camera;

struct SharedComponents {
    AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
    AnthemSimpleToyRenderer renderer;
    AnthemConfig config;

}shared;

struct ComputePipeline {
    using ssboPos = AtBufVecd2f<1>;
    using ssboVelocity = AtBufVecd2f<1>;
    using ssboColor = AtBufVecd4f<1>;
    using uniformDelta = AtUniformVecf<1>;

    AnthemDescriptorPool* descPoolSsbo;
    AnthemDescriptorPool* descPoolUniform;

    AnthemUniformBufferImpl<uniformDelta>* ubuf;
    AnthemShaderStorageBufferImpl<AnthemBufferVarDynamicDef<float, 1, 4, 1>>* ssbo;
    AnthemShaderFilePaths shaderFile;
    AnthemShaderModule* shader;
    AnthemComputePipeline* pipeline;

    AnthemFence** computeInFlight;
    AnthemSemaphore** computeFinish;
}comp;

struct DrawPipeline {
    using vxPosAttr = AtAttributeVecf<2>;
    using vxTexAttr = AtAttributeVecf<3>;

    AnthemVertexBufferImpl<vxPosAttr, vxTexAttr>* vxBuffer;
    AnthemIndexBuffer* ixBuffer;
    AnthemDepthBuffer* depthBuffer;
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;
    AnthemSwapchainFramebuffer* framebuffer;
}drw;

void prepareShared() {
    auto& renderer = shared.renderer;
    renderer.setConfig(&shared.config);
    renderer.initialize();
}

void prepareDraw() {
    auto& renderer = shared.renderer;
    renderer.createDepthBuffer(&drw.depthBuffer, false);

    renderer.createVertexBuffer(&drw.vxBuffer);
    drw.vxBuffer->setTotalVertices(4);
    drw.vxBuffer->insertData(0, { -1.0f, -1.0f }, { 1.0f, 0.0f });
    drw.vxBuffer->insertData(1, { 1.0f, -1.0f }, { 0.0f, 0.0f });
    drw.vxBuffer->insertData(2, { 1.0f, 1.0f }, { 0.0f, 1.0f });
    drw.vxBuffer->insertData(3, { -1.0f, 1.0f }, { 1.0f, 1.0f });

    renderer.createIndexBuffer(&drw.ixBuffer);
    drw.ixBuffer->setIndices({ 0,1,2,2,3,0 });

    //Create Pass
    renderer.setupDemoRenderPass(&drw.pass, drw.depthBuffer);
    renderer.createSwapchainImageFramebuffers(&drw.framebuffer, drw.pass, drw.depthBuffer);

    AnthemShaderFilePaths shaderFile = {
       .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\computeShaderDrawStage\\shader.vert.spv",
       .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\computeShaderDrawStage\\shader.frag.spv"
    };
    renderer.createShader(&drw.shader, &shaderFile);

    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { };
    renderer.createGraphicsPipelineCustomized(&drw.pipeline, descSetEntriesRegPipeline, drw.pass, drw.shader, drw.vxBuffer, nullptr);
    ANTH_LOGI("Done");
}

void prepareCompute() {
    auto& renderer = shared.renderer;
    renderer.createDescriptorPool(&comp.descPoolSsbo);
    renderer.createDescriptorPool(&comp.descPoolUniform);

    ANTH_LOGI("Creating Sync Fence");
    comp.computeInFlight = new std::remove_pointer_t<decltype(comp.computeInFlight)>[shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT];
    comp.computeFinish = new std::remove_pointer_t<decltype(comp.computeFinish)>[shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT];
    for (int i = 0; i < shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
        renderer.createFence(&comp.computeInFlight[i]);
        renderer.createSemaphore(&comp.computeFinish[i]);
    }
    
    ANTH_LOGI("Creating SSBO");
    std::function<void(decltype(comp.ssbo))> wFunc = [&](auto* p) {
        ANTH_LOGI("Invoked");
    };
    renderer.createShaderStorageBuffer(&comp.ssbo,4096, 0, comp.descPoolSsbo,std::make_optional(wFunc));
    
    
    ANTH_LOGI("Creating Uniform");
    renderer.createUniformBuffer(&comp.ubuf, 0, comp.descPoolUniform);

    comp.shaderFile.computeShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\computeShaderCompStage\\shader.comp.spv";
    renderer.createShader(&comp.shader, &comp.shaderFile);

    AnthemDescriptorSetEntry dseUniform = {
        .descPool = comp.descPoolUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry dseSsboIn = {
        .descPool = comp.descPoolSsbo,
        .descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry dseSsboOut = {
        .descPool = comp.descPoolSsbo,
        .descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { dseUniform,dseSsboIn,dseSsboOut };
    renderer.createComputePipelineCustomized(&comp.pipeline, descSetEntriesRegPipeline, comp.shader);
}

void recordCommandBufferDrw(int i) {
    auto& renderer = shared.renderer;
    renderer.drStartRenderPass(drw.pass, (AnthemFramebuffer*)(drw.framebuffer->getFramebufferObject(i)), i, false);
    renderer.drSetViewportScissor(i);
    renderer.drBindGraphicsPipeline(drw.pipeline, i);
    renderer.drBindVertexBuffer(drw.vxBuffer, i);
    renderer.drBindIndexBuffer(drw.ixBuffer, i);

    renderer.drDraw(drw.ixBuffer->getIndexCount(), i);
    renderer.drEndRenderPass(i);
}

void recordCommandBufferAll() {
    auto& renderer = shared.renderer;
    for (int i = 0; i < shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
        renderer.drStartCommandRecording(i);
        recordCommandBufferDrw(i);
        renderer.drEndCommandRecording(i);
    }
}

int main() {
    prepareShared();
    prepareDraw();
    prepareCompute();

    shared.renderer.registerPipelineSubComponents();
    recordCommandBufferAll();

    int currentFrame = 0;
    shared.renderer.setDrawFunction([&]() {
        uint32_t imgIdx;
        shared.renderer.drPrepareFrame(currentFrame, &imgIdx);
        shared.renderer.drSubmitBufferPrimaryCall(currentFrame, currentFrame);
        shared.renderer.drPresentFrame(currentFrame, imgIdx);
        currentFrame++;
        currentFrame %= 2;
        });
    shared.renderer.startDrawLoopDemo();
    shared.renderer.finalize();

    return 0;
}