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
    AnthemShaderStorageBufferImpl<ssboPos,ssboVelocity,ssboColor>* ssbo;
    AnthemShaderFilePaths shaderFile;
    AnthemShaderModule* shader;
    AnthemComputePipeline* pipeline;

    AnthemFence** computeInFlight;
    AnthemSemaphore** computeFinish;

    uint32_t* allocCmdBuf;

    using ssboType = decltype(ssbo);
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
    AnthemGraphicsPipelineCreateProps cprop;

}drw;

void prepareShared() {
    auto& renderer = shared.renderer;
    renderer.setConfig(&shared.config);
    renderer.initialize();
}

void prepareDrawLegacy() {
    auto& renderer = shared.renderer;

    renderer.createVertexBuffer(&drw.vxBuffer);
    drw.vxBuffer->setTotalVertices(4);
    float scaler = 0.5f;
    drw.vxBuffer->insertData(0, { -1.0f * scaler, -1.0f * scaler }, { 1.0f, 0.0f });
    drw.vxBuffer->insertData(1, { 1.0f * scaler, -1.0f * scaler }, { 0.0f, 0.0f });
    drw.vxBuffer->insertData(2, { 1.0f * scaler, 1.0f * scaler }, { 0.0f, 1.0f });
    drw.vxBuffer->insertData(3, { -1.0f * scaler, 1.0f * scaler }, { 1.0f, 1.0f });

    renderer.createIndexBuffer(&drw.ixBuffer);
    drw.ixBuffer->setIndices({ 0,1,2,2,3,0 });
}

void prepareDraw() {
    auto& renderer = shared.renderer;
    renderer.createDepthBuffer(&drw.depthBuffer, false);

    renderer.createIndexBuffer(&drw.ixBuffer);
    std::vector<uint32_t> indexList = {};
    for (uint32_t i = 0; i < 4096; i++) {
        indexList.push_back(i);
    }
    drw.ixBuffer->setIndices(indexList);

    //Create Pass
    renderer.setupDemoRenderPass(&drw.pass, drw.depthBuffer);
    renderer.createSwapchainImageFramebuffers(&drw.framebuffer, drw.pass, drw.depthBuffer);

    AnthemShaderFilePaths shaderFile = {
       .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\computeShaderDrawStage\\shader.vert.spv",
       .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\computeShaderDrawStage\\shader.frag.spv"
    };
    renderer.createShader(&drw.shader, &shaderFile);


    drw.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_POINT_LIST;

    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { };
    renderer.createGraphicsPipelineCustomized(&drw.pipeline, descSetEntriesRegPipeline, {}, drw.pass, drw.shader, comp.ssbo, &drw.cprop);
    ANTH_LOGI("Done");
}

void prepareSsboData(ComputePipeline::ssboType p) {
    for (int i = 0; i < 4096; i++) {
        float w = AnthemLinAlg::randomNumber<float>() * AT_PI * 2;
        float v = AnthemLinAlg::randomNumber<float>() * AT_PI * 2;
        float r = AnthemLinAlg::randomNumber<float>();
        float g = AnthemLinAlg::randomNumber<float>();
        float b = AnthemLinAlg::randomNumber<float>();

        p->setInput(i,
            { std::cos(w),std::sin(w) },
            { std::cos(v),std::sin(v) },
            { r,g,b,1.0f }
        );
    }
}

void prepareCompute() {
    auto& renderer = shared.renderer;
    renderer.createDescriptorPool(&comp.descPoolSsbo);
    renderer.createDescriptorPool(&comp.descPoolUniform);

    ANTH_LOGI("Allocating Command Buffer");
    comp.allocCmdBuf = new std::remove_pointer_t<decltype(comp.allocCmdBuf)>[shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT];
    for (int i = 0; i < shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
        renderer.drAllocateCommandBuffer(&comp.allocCmdBuf[i]);
    }

    ANTH_LOGI("Creating Sync Fence");
    comp.computeInFlight = new std::remove_pointer_t<decltype(comp.computeInFlight)>[shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT];
    comp.computeFinish = new std::remove_pointer_t<decltype(comp.computeFinish)>[shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT];
    for (int i = 0; i < shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
        renderer.createFence(&comp.computeInFlight[i]);
        renderer.createSemaphore(&comp.computeFinish[i]);
    }
    
    ANTH_LOGI("Creating SSBO");
    std::function<void(decltype(comp.ssbo))> wFunc = [&](decltype(comp.ssbo) p) {
        ANTH_LOGI("Invoked");
        prepareSsboData(p);
    };
    renderer.createShaderStorageBuffer(&comp.ssbo,4096, 0, comp.descPoolSsbo,std::make_optional(wFunc));
    
    
    ANTH_LOGI("Creating Uniform");
    renderer.createUniformBuffer(&comp.ubuf, 0, comp.descPoolUniform);
    float ubufData = 0.0005;
    comp.ubuf->specifyUniforms(&ubufData);
    for (int i = 0; i < shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT;i++) {
        comp.ubuf->updateBuffer(i);
    }
    

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
    renderer.drSetViewportScissorFromSwapchain(i);
    renderer.drBindGraphicsPipeline(drw.pipeline, i);
    renderer.drBindVertexBufferFromSsbo(comp.ssbo,0, i);
    renderer.drBindIndexBuffer(drw.ixBuffer, i);

    renderer.drDraw(drw.ixBuffer->getIndexCount(), i);
    renderer.drEndRenderPass(i);
}

void recordCommandBufferComp(int i) {
    auto& renderer = shared.renderer;
    renderer.drBindComputePipeline(comp.pipeline, comp.allocCmdBuf[i]);
    AnthemDescriptorSetEntry dseUniform = {
        .descPool = comp.descPoolUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry dseSsboIn = {
        .descPool = comp.descPoolSsbo,
        .descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
        .inTypeIndex = static_cast<uint32_t>(i)
    };
    AnthemDescriptorSetEntry dseSsboOut = {
        .descPool = comp.descPoolSsbo,
        .descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
        .inTypeIndex = (static_cast<uint32_t>(i) + 1) % 2
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { dseUniform,dseSsboIn,dseSsboOut };
    renderer.drBindDescriptorSetCustomizedCompute(descSetEntriesRegPipeline, comp.pipeline, comp.allocCmdBuf[i]);
    renderer.drComputeDispatch(comp.allocCmdBuf[i], 128, 1, 1);
}

void recordCommandBufferCompAll() {
    auto& renderer = shared.renderer;
    for (int i = 0; i < shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
        renderer.drStartCommandRecording(comp.allocCmdBuf[i]);
        ANTH_LOGI("Start Recording:", comp.allocCmdBuf[i]);
        recordCommandBufferComp(i);
        renderer.drEndCommandRecording(comp.allocCmdBuf[i]);
    }
}

void recordCommandBufferAll() {
    auto& renderer = shared.renderer;
    for (int i = 0; i < shared.config.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
        renderer.drStartCommandRecording(i);
        recordCommandBufferDrw(i);
        renderer.drEndCommandRecording(i);
    }
}

void drawLoop(int& currentFrame) {

    uint32_t imgIdx;
    shared.renderer.drPrepareFrame(currentFrame, &imgIdx);

    // Compute Task
    comp.computeInFlight[currentFrame]->waitForFence();
    comp.computeInFlight[currentFrame]->resetFence();

    std::vector<const AnthemSemaphore*> semaphoreToSignal = { comp.computeFinish[currentFrame] };
    std::vector<AtSyncSemaphoreWaitStage> waitStage = { AtSyncSemaphoreWaitStage::AT_SSW_VERTEX_INPUT };
    shared.renderer.drSubmitCommandBufferCompQueueGeneral(comp.allocCmdBuf[currentFrame], nullptr, &semaphoreToSignal, comp.computeInFlight[currentFrame]);

    // Graphics Drawing
    //shared.renderer.drSubmitBufferPrimaryCall(currentFrame, currentFrame);
    shared.renderer.drSubmitCommandBufferGraphicsQueueGeneral(currentFrame, imgIdx, &semaphoreToSignal, &waitStage);
    shared.renderer.drPresentFrame(currentFrame, imgIdx);
    currentFrame++;
    currentFrame %= 2;
}

int main() {
    prepareShared();
    prepareCompute();
    prepareDraw();

    shared.renderer.registerPipelineSubComponents();
    recordCommandBufferAll();
    recordCommandBufferCompAll();

    int currentFrame = 0;
    shared.renderer.setDrawFunction([&]() {
        drawLoop(currentFrame);
    });
    shared.renderer.startDrawLoopDemo();
    shared.renderer.finalize();

    return 0;
}