#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::External;

struct TempCamera{
    AtMatf4 projection;    
    AtMatf4 view;
    AtMatf4 model;
    AtMatf4 viewspaceNormal;
};

struct AOParams{
    constexpr static int sampleVecDim = 4;
    constexpr static int noiseVecDim = 4;
    constexpr static int noiseTexSize = 8;
    constexpr static int numSamples = 128;

    float* samples;  
    float* noise;
    uint8_t* noiseU;
} aoParam;

struct DeferPass{
    using vxPosAttr = AtAttributeVecf<3>;
    using vxNormalAttr = AtAttributeVecf<3>;

    using uxProjMat = AnthemUniformMatf<4>;
    using uxModelMat = AnthemUniformMatf<4>;
    using uxViewMat = AnthemUniformMatf<4>;
    using uxNormalMat = AnthemUniformMatf<4>;

    AnthemDescriptorPool* descPoolUniform;
    AnthemDescriptorPool* descPoolPositionAtt;
    AnthemDescriptorPool* descPoolNormalAtt;

    AnthemVertexBufferImpl<vxPosAttr,vxNormalAttr>** vxBuffers;
    AnthemIndexBuffer** ixBuffers;
    AnthemUniformBufferImpl<uxProjMat,uxViewMat,uxModelMat,uxNormalMat>* ubuf;

    AnthemDepthBuffer* depthBuffer;
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;
    AnthemFramebuffer* framebuffer;

    AnthemImage* normalImage;
    AnthemImage* positionImage;

    const int numMeshes = 4;
};

struct AOPass{
    using vxPosAttr = AtAttributeVecf<2>;
    using vxTexAttr = AtAttributeVecf<3>;

    using uxDeferProjection = AtUniformMatf<4>;
    using uxSamples = AtUniformVecfArray<4,aoParam.numSamples>;
    using uxWindowsState = AtUniformVecf<4>;

    AnthemVertexBufferImpl<vxPosAttr,vxTexAttr>* vxBuffer;
    AnthemUniformBufferImpl<uxDeferProjection,uxWindowsState,uxSamples>* uxBuffer;
    AnthemIndexBuffer* ixBuffer;
    AnthemDepthBuffer* depthBuffer;
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;
    AnthemSwapchainFramebuffer* framebuffer;

    AnthemDescriptorPool* descPoolNoise;
    AnthemImage* aoNoise;

    AnthemDescriptorPool* descPoolUniform;
};

void prepareAOPipeline(AOPass& target, DeferPass& offscr, AnthemSimpleToyRenderer& renderer){
    //Creating Descriptor Pool
    renderer.createDescriptorPool(&target.descPoolNoise);
    renderer.createDescriptorPool(&target.descPoolUniform);

    //Creating Vertex Buffer & Index Buffer
    renderer.createVertexBuffer(&target.vxBuffer);
    target.vxBuffer->setTotalVertices(4);
    target.vxBuffer->insertData(0,{-1.0f, -1.0f}, {1.0f, 0.0f});
    target.vxBuffer->insertData(1,{1.0f, -1.0f}, {0.0f, 0.0f});
    target.vxBuffer->insertData(2,{1.0f, 1.0f}, {0.0f, 1.0f});
    target.vxBuffer->insertData(3,{-1.0f, 1.0f}, {1.0f, 1.0f});

    renderer.createIndexBuffer(&target.ixBuffer);
    target.ixBuffer->setIndices({0,1,2,2,3,0});

    //Creating Uniform Buffer
    renderer.createUniformBuffer(&target.uxBuffer,0,target.descPoolUniform);

    //Load Image
    renderer.createTexture(&target.aoNoise,target.descPoolNoise,aoParam.noiseU,aoParam.noiseTexSize,aoParam.noiseTexSize,aoParam.noiseVecDim,0,false,false);

    //Depth Buffer
    renderer.createDepthBuffer(&target.depthBuffer,false);

    //Create Pass
    renderer.setupDemoRenderPass(&target.pass,target.depthBuffer);
    renderer.createSwapchainImageFramebuffers(&target.framebuffer,target.pass,target.depthBuffer);

    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\ssaoComposition\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\ssaoComposition\\shader.frag.spv"
    };
    renderer.createShader(&target.shader,&shaderFile);

    //Create Pipeline
    AnthemDescriptorSetEntry samplerNormal = {
        .descPool = offscr.descPoolNormalAtt,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerPosition = {
        .descPool = offscr.descPoolPositionAtt,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerAONoise = {
        .descPool = target.descPoolNoise,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry uniformAO = {
        .descPool = target.descPoolUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {samplerNormal,samplerPosition,samplerAONoise,uniformAO};
    renderer.createGraphicsPipelineCustomized(&target.pipeline,descSetEntriesRegPipeline,target.pass,target.shader,target.vxBuffer);
    ANTH_LOGI("Done");
}

void prepareOffscreen(DeferPass& offscreen,AnthemSimpleToyRenderer& renderer){
    // Loading Model
    AnthemGLTFLoader loader;
    AnthemGLTFLoaderParseConfig gltfConfig;
    std::vector<AnthemGLTFLoaderParseResult> gltfResult;
    loader.loadModel("C:\\WR\\Aria\\Anthem\\assets\\gsk\\untitled.gltf");
    loader.parseModel(gltfConfig,gltfResult);
    ANTH_LOGI("Model Loaded");

    //Creating Descriptor Pool
    renderer.createDescriptorPool(&offscreen.descPoolPositionAtt);
    renderer.createDescriptorPool(&offscreen.descPoolNormalAtt);
    renderer.createDescriptorPool(&offscreen.descPoolUniform);

    //Creating Vertex Buffer & Index Buffer
    offscreen.vxBuffers = new std::remove_pointer_t<decltype(offscreen.vxBuffers)>[gltfResult.size()];
    offscreen.ixBuffers = new AnthemIndexBuffer*[gltfResult.size()];
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        renderer.createVertexBuffer(&offscreen.vxBuffers[chosenMesh]);
        float dfz = 0.1f;
        float dpz = 1.0f;
        int numVertices = gltfResult.at(chosenMesh).positions.size()/3;
        offscreen.vxBuffers[chosenMesh]->setTotalVertices(numVertices);
        for(int i=0;i<numVertices;i++){
            offscreen.vxBuffers[chosenMesh]->insertData(i,
                {gltfResult.at(chosenMesh).positions.at(i*3),-gltfResult.at(chosenMesh).positions.at(i*3+1),gltfResult.at(chosenMesh).positions.at(i*3+2)},
                {gltfResult.at(chosenMesh).normals.at(i*3),-gltfResult.at(chosenMesh).normals.at(i*3+1),gltfResult.at(chosenMesh).normals.at(i*3+2)}
            );
        }
        ANTH_LOGI("Vertex Buffer Created");
        renderer.createIndexBuffer(&offscreen.ixBuffers[chosenMesh]);
        std::vector<uint32_t> indices;
        for(int i=0;i<gltfResult.at(chosenMesh).indices.size();i++){
            indices.push_back(gltfResult.at(chosenMesh).indices.at(i));
        }
        offscreen.ixBuffers[chosenMesh]->setIndices(indices);
        ANTH_LOGI("Index Buffer Created");
    }

    //Create Uniform Buffer
    renderer.createUniformBuffer(&offscreen.ubuf,0,offscreen.descPoolUniform);

    //Create Depth Buffer
    renderer.createDepthBuffer(&offscreen.depthBuffer,false);

    //Create Pass
    AnthenRenderPassSetupOption setupOpt{
        .renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS,
        .msaaType = AT_ARPMT_NO_MSAA,
        .colorAttachmentFormats = { AT_IF_SRGB_FLOAT32, AT_IF_SRGB_FLOAT32 }
    };
    renderer.setupRenderPass(&offscreen.pass,&setupOpt,offscreen.depthBuffer);
    ANTH_LOGI("Render Pass Created");

    //Create Attachment
    renderer.createColorAttachmentImage(&offscreen.positionImage, offscreen.descPoolPositionAtt, 0, AT_IF_SRGB_FLOAT32,false);
    renderer.createColorAttachmentImage(&offscreen.normalImage, offscreen.descPoolNormalAtt, 0,AT_IF_SRGB_FLOAT32,false);
    ANTH_LOGI("Attachments Created");

    //Create Framebuffer
    std::vector<const AnthemImage*> tmpColorAttachment = {offscreen.positionImage, offscreen.normalImage };
    renderer.createSimpleFramebuffer(&offscreen.framebuffer,&tmpColorAttachment,offscreen.pass,offscreen.depthBuffer);
    ANTH_LOGI("Framebuffer Created");

    //Create Shader
    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\ssaoDefer\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\ssaoDefer\\shader.frag.spv"
    };
    renderer.createShader(&offscreen.shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    //Assemble Pipeline
    AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
        .descPool = offscreen.descPoolUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {uniformBufferDescEntryRegPipeline};

    renderer.createGraphicsPipelineCustomized(&offscreen.pipeline,descSetEntriesRegPipeline,offscreen.pass,offscreen.shader,offscreen.vxBuffers[0]);
    ANTH_LOGI("Pipeline Created");
}

void updateOffscrUniform(DeferPass& offscr,AOPass& target ,AnthemSimpleToyRenderer& renderer,int currentFrame){
    int rdWinH,rdWinW;
    renderer.exGetWindowSize(rdWinH,rdWinW);
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransformWithFovAspect(0.1f,300.0f,(float)AT_PI/2.0f,1.0f*rdWinW/rdWinH);
    auto axis = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,-70.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,-70.0f,-80.0f});
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto lookAt = Math::AnthemLinAlg::lookAtTransform(eye,center,up);
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*0.5);
    
    auto modelview = lookAt.multiply(local);

    auto modelrot = lookAt.clipSubmatrixLeftTop<3,3>();
    auto localrot = local.clipSubmatrixLeftTop<3,3>();
    auto modelviewrot = modelrot.multiply(localrot);
    auto normalMat = Math::AnthemLinAlg::inverse3(modelviewrot).transpose().padRightBottom<1,1>();

    float projMatVal[16];
    float viewMatVal[16];
    float modelMatVal[16];
    float normalMatVal[16];
    float windowState[4] = {rdWinH*1.0f,rdWinW*1.0f,aoParam.noiseTexSize,1};
    proj.columnMajorVectorization(projMatVal);
    lookAt.columnMajorVectorization(viewMatVal);
    local.columnMajorVectorization(modelMatVal);
    normalMat.columnMajorVectorization(normalMatVal);

    offscr.ubuf->specifyUniforms(projMatVal,viewMatVal,modelMatVal,normalMatVal);
    offscr.ubuf->updateBuffer(currentFrame);

    target.uxBuffer->specifyUniforms(projMatVal,windowState,aoParam.samples);
    target.uxBuffer->updateBuffer(currentFrame);
}

void generateAOParameters(){
    //Generate Samples & Noises
    using AOParamTp = std::remove_pointer_t<decltype(aoParam.samples)>;
    decltype(aoParam.samples)* vecContainer[] = {&aoParam.samples,&aoParam.noise};
    const int vecDim[] = {aoParam.sampleVecDim, aoParam.noiseVecDim};
    const int vecNum[] = {aoParam.numSamples, aoParam.noiseVecDim * aoParam.noiseVecDim};
    for(int T=0;T<2;T++){
        *(vecContainer[T]) = new float[vecDim[T]*vecNum[T]];
        for(auto i=0;i<vecNum[T];i++){
            const auto d = (T==0)?AnthemLinAlg::randomVector3<AOParamTp>():AnthemLinAlg::randomVectorIid3<AOParamTp>();
            for(auto j=0;j<3;j++){
                auto& p = (*(vecContainer[T]))[i*4+j];
                p = d[j];
                if(T==0&&j==2) p = std::abs(p);
                if(T==1&&j==2) p = 0;
                if(T==0){
                    auto wx = AnthemLinAlg::randomNumber<float>();
                    p *=(wx*wx);
                }
            }
        }
    }

    //Clamp Noises
    aoParam.noiseU = new std::remove_pointer_t<decltype(aoParam.noiseU)>[vecDim[1]*vecNum[1]];
    for(int i=0;i<vecDim[1]*vecNum[1];i++){
        aoParam.noiseU[i] = (aoParam.noise[i] + 1.0)/2.0 * 255;
    }
}

void recordCommandsOffscreen(AnthemConfig* cfg,AnthemSimpleToyRenderer& renderer, DeferPass& offscreen,int i){
    //Prepare Command
    renderer.drStartRenderPass(offscreen.pass,(AnthemFramebuffer *)(offscreen.framebuffer),i,false);
    renderer.drSetViewportScissor(i);
    renderer.drBindGraphicsPipeline(offscreen.pipeline,i);
    for(int j=0;j<offscreen.numMeshes;j++){
        AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
            .descPool = offscreen.descPoolUniform,
            .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
            .inTypeIndex = 0
        };
        std::vector<AnthemDescriptorSetEntry> descSetEntries = {uniformBufferDescEntryRdw};
        renderer.drBindVertexBuffer(offscreen.vxBuffers[j],i);
        renderer.drBindIndexBuffer(offscreen.ixBuffers[j],i);
        renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries,offscreen.pipeline,i);
        renderer.drDraw(offscreen.ixBuffers[j]->getIndexCount(),i);
    }
    renderer.drEndRenderPass(i);
}
void recordCommandsTarget(AnthemConfig* cfg,AnthemSimpleToyRenderer& renderer, AOPass& target,DeferPass& offscreen,int i){
    renderer.drStartRenderPass(target.pass,(AnthemFramebuffer *)(target.framebuffer->getFramebufferObject(i)),i,false);
    renderer.drSetViewportScissor(i);
    renderer.drBindGraphicsPipeline(target.pipeline,i);

    AnthemDescriptorSetEntry samplerNormal = {
        .descPool = offscreen.descPoolNormalAtt,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerPosition = {
        .descPool = offscreen.descPoolPositionAtt,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerAONoise = {
        .descPool = target.descPoolNoise,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry uniformAO = {
        .descPool = target.descPoolUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {samplerNormal,samplerPosition,samplerAONoise,uniformAO};
    renderer.drBindVertexBuffer(target.vxBuffer,i);
    renderer.drBindIndexBuffer(target.ixBuffer,i);
    renderer.drBindDescriptorSetCustomizedGraphics(descSetEntriesRegPipeline,target.pipeline,i);
    renderer.drDraw(target.ixBuffer->getIndexCount(),i);
    renderer.drEndRenderPass(i);
}

void recordCommandsAll(AnthemConfig* cfg,AnthemSimpleToyRenderer& renderer, DeferPass& offscreen,AOPass& target){
    for(int i=0;i<cfg->VKCFG_MAX_IMAGES_IN_FLIGHT;i++){
        renderer.drClearCommands(i);
        renderer.drStartCommandRecording(i);
        ANTH_LOGI("First");
        recordCommandsOffscreen(cfg, renderer, offscreen,i);
        ANTH_LOGI("Second");
        recordCommandsTarget(cfg, renderer, target,offscreen,i);
        renderer.drEndCommandRecording(i);
    }
}



int main(){
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto renderer = ANTH_MAKE_SHARED(Anthem::Core::AnthemSimpleToyRenderer)();
    renderer->setConfig(cfg.get());
    renderer->initialize();
    ANTH_LOGI("Intialization Complete");

    generateAOParameters();
    ANTH_LOGI("AO Params Generated");

    DeferPass offscr;
    AOPass target;
    prepareOffscreen(offscr,*renderer.get());
    prepareAOPipeline(target,offscr,*renderer.get());

    //Start Loop
    ANTH_LOGI("Start Reg");
    renderer->registerPipelineSubComponents();

    ANTH_LOGI("Start Rec Commands");
    recordCommandsAll(cfg.get(),*renderer.get(),offscr,target);

    //START!
    int currentFrame = 0;
    renderer->setDrawFunction([&](){
        updateOffscrUniform(offscr,target,*renderer.get(),currentFrame);
        uint32_t imgIdx;
        renderer->drPrepareFrame(currentFrame,&imgIdx);
        renderer->drSubmitBufferPrimaryCall(currentFrame, currentFrame);
        renderer->drPresentFrame(currentFrame,imgIdx);
        currentFrame++;
        currentFrame %= 2;
    });
    ANTH_LOGI("Loop Started");
    renderer->startDrawLoopDemo();
    renderer->finalize();

    return 0;
}