#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/components/camera/AnthemCamera.h"

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::External;
using namespace Anthem::Components::Camera;

struct SharedComponents{
    AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
    AnthemCamera lightPov = AnthemCamera(AT_ACPT_ORTHO);
    AnthemSimpleToyRenderer renderer;
    AnthemConfig config;
    AtVecf4 lightDir;
}shared;

struct OffscreenPass{
    constexpr static int numMeshes = 5;
    using vxPosAttr = AtAttributeVecf<3>;
    using vxNormalAttr = AtAttributeVecf<3>;

    using uxProjMat = AnthemUniformMatf<4>;
    using uxModelMat = AnthemUniformMatf<4>;
    using uxViewMat = AnthemUniformMatf<4>;
    using uxNormalMat = AnthemUniformMatf<4>;

    AnthemDescriptorPool* descPoolUniform;
    AnthemDescriptorPool* descPoolDepth;

    AnthemVertexBufferImpl<vxPosAttr,vxNormalAttr>** vxBuffers;
    AnthemIndexBuffer** ixBuffers;
    AnthemUniformBufferImpl<uxProjMat,uxViewMat,uxModelMat,uxNormalMat>* ubuf;

    AnthemDepthBuffer* depthBuffer;
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;

    AnthemFramebuffer* framebuffer;
}offscreenPass;

struct MainPass{
    using uxProjMat = AnthemUniformMatf<4>;
    using uxModelMat = AnthemUniformMatf<4>;
    using uxViewMat = AnthemUniformMatf<4>;
    using uxNormalMat = AnthemUniformMatf<4>;
    using uxLightPovProjMat = AtUniformMatf<4>;
    using uxLightPovViewMat = AtUniformMatf<4>;
    using uxLightDirection = AtUniformVecf<4>;
    using uxShadowMapSize = AtUniformVecf<4>;
    
    AnthemDescriptorPool* descPoolUniform;
    AnthemUniformBufferImpl<uxProjMat,uxViewMat,uxModelMat,uxNormalMat,uxLightPovProjMat,uxLightPovViewMat,uxLightDirection,uxShadowMapSize>* ubuf;

    AnthemDepthBuffer* depthBuffer;
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;

    AnthemSwapchainFramebuffer* framebuffer;

}targetPass;

struct DebugPass{
    using vxPosAttr = AtAttributeVecf<2>;
    using vxTexAttr = AtAttributeVecf<3>;

    using uxDeferProjection = AtUniformMatf<4>;

    AnthemVertexBufferImpl<vxPosAttr,vxTexAttr>* vxBuffer;
    AnthemUniformBufferImpl<uxDeferProjection>* uxBuffer;
    AnthemIndexBuffer* ixBuffer;
    AnthemDepthBuffer* depthBuffer;
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;
    AnthemSwapchainFramebuffer* framebuffer;

    AnthemDescriptorPool* descPoolUniform;
}debugPass;

void prepareDebug(){
    auto& renderer = shared.renderer;
    auto& shadow = offscreenPass;
    auto& target = debugPass;

    //Creating Descriptor Pool
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

    //Depth Buffer
    renderer.createDepthBuffer(&target.depthBuffer,false);

    //Create Pass
    renderer.setupDemoRenderPass(&target.pass,target.depthBuffer);
    renderer.createSwapchainImageFramebuffers(&target.framebuffer,target.pass,target.depthBuffer);

    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\shadowMappingDirectionalDebug\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\shadowMappingDirectionalDebug\\shader.frag.spv"
    };
    renderer.createShader(&target.shader,&shaderFile);

    //Create Pipeline
    AnthemDescriptorSetEntry samplerPosition = {
        .descPool = shadow.descPoolDepth,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };

    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {samplerPosition};
    renderer.createGraphicsPipelineCustomized(&target.pipeline,descSetEntriesRegPipeline, {}, target.pass,target.shader,target.vxBuffer,nullptr);
    ANTH_LOGI("Done");
}


void prepareTarget(){
    auto& renderer = shared.renderer;
    auto& shadow = offscreenPass;
    auto& target = targetPass;

    //Creating Descriptor Pool
    renderer.createDescriptorPool(&target.descPoolUniform);

    //Create Uniform Buffer
    renderer.createUniformBuffer(&target.ubuf,0,target.descPoolUniform);

    //Create Depth Buffer
    renderer.createDepthBuffer(&target.depthBuffer,false);

    //Create Pass
    AnthemRenderPassSetupOption setupOpt{
        .renderPassUsage = AT_ARPAA_FINAL_PASS,
        .msaaType = AT_ARPMT_NO_MSAA,
        .colorAttachmentFormats = { std::nullopt }
    };
    renderer.setupRenderPass(&target.pass,&setupOpt,target.depthBuffer);
    renderer.createSwapchainImageFramebuffers(&target.framebuffer,target.pass,target.depthBuffer);
    ANTH_LOGI("Render Pass Created");

    //Create Shader
    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\shadowMappingDirectionalAfter\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\shadowMappingDirectionalAfter\\shader.frag.spv",
    };
    renderer.createShader(&target.shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    //Assemble Pipeline
    AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
        .descPool = target.descPoolUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerShadowDepthInfo = {
        .descPool = shadow.descPoolDepth,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {uniformBufferDescEntryRegPipeline,samplerShadowDepthInfo};

    renderer.createGraphicsPipelineCustomized(&target.pipeline,descSetEntriesRegPipeline, {}, target.pass,target.shader,shadow.vxBuffers[0], nullptr);
    ANTH_LOGI("Pipeline Created");
}

void prepareOffscreen(){
    auto& renderer = shared.renderer;
    auto& target = offscreenPass;

    // Loading Model
    AnthemGLTFLoader loader;
    AnthemGLTFLoaderParseConfig gltfConfig;
    std::vector<AnthemGLTFLoaderParseResult> gltfResult;
    loader.loadModel("C:\\WR\\Aria\\Anthem\\assets\\gsk\\untitled.gltf");
    loader.parseModel(gltfConfig,gltfResult);
    ANTH_LOGI("Model Loaded");

    //Creating Descriptor Pool
    renderer.createDescriptorPool(&target.descPoolUniform);
    renderer.createDescriptorPool(&target.descPoolDepth);

    //Creating Vertex Buffer & Index Buffer
    target.vxBuffers = new std::remove_pointer_t<decltype(target.vxBuffers)>[target.numMeshes];
    target.ixBuffers = new AnthemIndexBuffer*[target.numMeshes];

    // -> Model
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        renderer.createVertexBuffer(&target.vxBuffers[chosenMesh]);
        float dfz = 0.1f;
        float dpz = 1.0f;
        int numVertices = gltfResult.at(chosenMesh).positions.size()/3;
        target.vxBuffers[chosenMesh]->setTotalVertices(numVertices);
        for(int i=0;i<numVertices;i++){
            target.vxBuffers[chosenMesh]->insertData(i,
                {gltfResult.at(chosenMesh).positions.at(i*3),gltfResult.at(chosenMesh).positions.at(i*3+1),gltfResult.at(chosenMesh).positions.at(i*3+2)},
                {gltfResult.at(chosenMesh).normals.at(i*3),gltfResult.at(chosenMesh).normals.at(i*3+1),gltfResult.at(chosenMesh).normals.at(i*3+2)}
            );
        }
        ANTH_LOGI("Vertex Buffer Created");
        renderer.createIndexBuffer(&target.ixBuffers[chosenMesh]);
        std::vector<uint32_t> indices;
        for(int i=0;i<gltfResult.at(chosenMesh).indices.size();i++){
            indices.push_back(gltfResult.at(chosenMesh).indices.at(i));
        }
        target.ixBuffers[chosenMesh]->setIndices(indices);
        ANTH_LOGI("Index Buffer Created");
    }
    // -> Platform
    constexpr static float bw = 120.0;
    constexpr static float bx = 120.0;
    constexpr static float by = 5.0;
    
    renderer.createVertexBuffer(&target.vxBuffers[target.numMeshes-1]);
    target.vxBuffers[target.numMeshes-1]->setTotalVertices(4);
    target.vxBuffers[target.numMeshes-1]->insertData(0,{bx,by,bw},{0.0f,1.0f,0.0f});
    target.vxBuffers[target.numMeshes-1]->insertData(1,{-bx,by,bw},{0.0f,1.0f,0.0f});
    target.vxBuffers[target.numMeshes-1]->insertData(2,{-bx,by,-bw},{0.0f,1.0f,0.0f});
    target.vxBuffers[target.numMeshes-1]->insertData(3,{bx,by,-bw},{0.0f,1.0f,0.0f});

    renderer.createIndexBuffer(&target.ixBuffers[target.numMeshes-1]);
    target.ixBuffers[target.numMeshes-1]->setIndices(std::vector<uint32_t>({0,1,2,2,3,0}));

    //Create Uniform Buffer
    renderer.createUniformBuffer(&target.ubuf,0,target.descPoolUniform);

    //Create Depth Buffer
    ANTH_LOGI("Creating Depth Buffer");
    renderer.createDepthBufferWithSampler(&target.depthBuffer,target.descPoolDepth,0,false);

    //Create Pass
    ANTH_LOGI("Creating Render Pass");
    AnthemRenderPassSetupOption setupOpt{
        .renderPassUsage = AT_ARPAA_DEPTH_STENCIL_ONLY_PASS,
        .msaaType = AT_ARPMT_NO_MSAA,
        .colorAttachmentFormats = { }
    };
    renderer.setupRenderPass(&target.pass,&setupOpt,target.depthBuffer);
    ANTH_LOGI("Render Pass Created");

    const std::vector<const IAnthemImageViewContainer*> temp = {};
    renderer.createSimpleFramebuffer(&target.framebuffer,&temp,target.pass,target.depthBuffer);

    //Create Shader
    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\shadowMappingDirectional\\shader.vert.spv",
    };
    renderer.createShader(&target.shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    //Assemble Pipeline
    AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
        .descPool = target.descPoolUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {uniformBufferDescEntryRegPipeline};

    renderer.createGraphicsPipelineCustomized(&target.pipeline,descSetEntriesRegPipeline, {}, target.pass,target.shader,target.vxBuffers[0], nullptr);
    ANTH_LOGI("Pipeline Created");

}

void recordCommandsShadowCalcStage(int i){
    auto& target = offscreenPass;
    auto& renderer = shared.renderer;
    auto& cfg = shared.config;
    //Prepare Command
    renderer.drStartRenderPass(target.pass,(AnthemFramebuffer *)(target.framebuffer),i,false);
    renderer.drSetViewportScissor(i);
    renderer.drBindGraphicsPipeline(target.pipeline,i);
    for(int j=0;j<target.numMeshes;j++){
        AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
            .descPool = target.descPoolUniform,
            .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
            .inTypeIndex = 0
        };

        std::vector<AnthemDescriptorSetEntry> descSetEntries = {uniformBufferDescEntryRdw};
        renderer.drBindVertexBuffer(target.vxBuffers[j],i);
        renderer.drBindIndexBuffer(target.ixBuffers[j],i);
        renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries,target.pipeline,i);
        renderer.drDraw(target.ixBuffers[j]->getIndexCount(),i);
    }
    renderer.drEndRenderPass(i);
}

void recordCommandMainStage(int i){
    auto& target = targetPass;
    auto& shadow = offscreenPass;
    auto& renderer = shared.renderer;
    auto& cfg = shared.config;
    renderer.drStartRenderPass(target.pass,(AnthemFramebuffer *)(target.framebuffer->getFramebufferObject(i)),i,false);
    renderer.drSetViewportScissor(i);
    renderer.drBindGraphicsPipeline(target.pipeline,i);
    for(int j=0;j<shadow.numMeshes;j++){
        AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
            .descPool = target.descPoolUniform,
            .descSetType = AT_ACDS_UNIFORM_BUFFER,
            .inTypeIndex = 0
        };
        AnthemDescriptorSetEntry samplerShadowDepthInfo = {
            .descPool = shadow.descPoolDepth,
            .descSetType = AT_ACDS_SAMPLER,
            .inTypeIndex = 0
        };
    
        std::vector<AnthemDescriptorSetEntry> descSetEntries = {uniformBufferDescEntryRdw,samplerShadowDepthInfo};
        renderer.drBindVertexBuffer(shadow.vxBuffers[j],i);
        renderer.drBindIndexBuffer(shadow.ixBuffers[j],i);
        renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries,target.pipeline,i);
        renderer.drDraw(shadow.ixBuffers[j]->getIndexCount(),i);
    }
    renderer.drEndRenderPass(i);
}

void recordCommandDebugStage(int i){
    auto& target = debugPass;
    auto& shadow = offscreenPass;
    auto& renderer = shared.renderer;
    auto& cfg = shared.config;
    renderer.drStartRenderPass(target.pass,(AnthemFramebuffer *)(target.framebuffer->getFramebufferObject(i)),i,false);
    renderer.drSetViewportScissor(i);
    renderer.drBindGraphicsPipeline(target.pipeline,i);

    AnthemDescriptorSetEntry samplerShadowDepthInfo = {
        .descPool = shadow.descPoolDepth,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntries = {samplerShadowDepthInfo};
    renderer.drBindVertexBuffer(target.vxBuffer,i);
    renderer.drBindIndexBuffer(target.ixBuffer,i);
    renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries,target.pipeline,i);
    renderer.drDraw(target.ixBuffer->getIndexCount(),i);
    renderer.drEndRenderPass(i);
}
void recordCommandsAll(){
    auto& target = offscreenPass;
    auto& renderer = shared.renderer;
    auto& cfg = shared.config;
    for(int i=0;i<cfg.VKCFG_MAX_IMAGES_IN_FLIGHT;i++){
        renderer.drClearCommands(i);
        renderer.drStartCommandRecording(i);
        recordCommandsShadowCalcStage(i);
        recordCommandMainStage(i);
        //recordCommandDebugStage(i);
        renderer.drEndCommandRecording(i);
    }
}

void prepareSharedComponents(){
    int rdH,rdW;
    shared.renderer.setConfig(&shared.config);
    shared.renderer.initialize();
    shared.renderer.exGetWindowSize(rdH,rdW);
    shared.camera.specifyFrustum((float)AT_PI/2.0f,0.1f,500.0f,1.0f*rdW/rdH);
    shared.camera.specifyPosition(0.0f,70.0f,-80.0f);

    shared.lightPov.specifyFrustum((float)AT_PI/2.0f,0.1f,500.0f,1.0f*rdW/rdH);
    shared.lightPov.specifyOrthoClipSpace(0.1f,300.0f,(float)AT_PI/2.0f,100.0f);
    shared.lightPov.specifyPosition(80.0f,140.0f,-80.0f);
    shared.lightPov.specifyFrontEyeRay(-1.0f,-1.0f,1.0f);

    shared.lightDir = {-1.0f,-1.0f,1.0f,1.0f};
    ANTH_LOGI("Intialization Complete");
}


void updateOffscrUniform(int currentFrame){
    auto& target = offscreenPass;
    auto& target2 = targetPass;
    auto& renderer = shared.renderer;

    int rdWinH,rdWinW;
    renderer.exGetWindowSize(rdWinH,rdWinW);
    auto axis = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*0.3);
    auto localrot = local.clipSubmatrixLeftTop<3,3>();
    
    AtMatf4 lightPovProj,lightPovView;
    shared.lightPov.getProjectionMatrix(lightPovProj);
    shared.lightPov.getViewMatrix(lightPovView);

    AtMatf4 camPovProj,camPovView;
    shared.camera.getProjectionMatrix(camPovProj);
    shared.camera.getViewMatrix(camPovView);

    auto lightViewRot = lightPovView.clipSubmatrixLeftTop<3,3>();
    auto lightMvRot = lightViewRot.multiply(localrot);
    auto lightNormalMat = Math::AnthemLinAlg::inverse3(lightMvRot).transpose().padRightBottom<1,1>();
    auto modelInvNormal = Math::AnthemLinAlg::inverse3(localrot).transpose().padRightBottom<1,1>();
    
    float lightProjMatVal[16];
    float lightViewMatVal[16];
    float lightNormalMatVal[16];
    float camProjMatVal[16];
    float camViewMatVal[16];
    float camNormalMatVal[16];
    float modelMatVal[16];
    float lightDir[4];
    float shadowMapParamVal[4];

    lightPovProj.columnMajorVectorization(lightProjMatVal);
    lightPovView.columnMajorVectorization(lightViewMatVal);
    lightNormalMat.columnMajorVectorization(lightNormalMatVal);
    camPovProj.columnMajorVectorization(camProjMatVal);
    camPovView.columnMajorVectorization(camViewMatVal);
    modelInvNormal.columnMajorVectorization(camNormalMatVal);
    local.columnMajorVectorization(modelMatVal);
    shared.lightDir.vectorization(lightDir);
    shadowMapParamVal[0] = targetPass.depthBuffer->getImageWidth();
    shadowMapParamVal[1] = targetPass.depthBuffer->getImageHeight();
    shadowMapParamVal[2] = 0;
    shadowMapParamVal[3] = 0;

    target.ubuf->specifyUniforms(lightProjMatVal,lightViewMatVal,modelMatVal,lightNormalMatVal);
    target.ubuf->updateBuffer(currentFrame);
    
    target2.ubuf->specifyUniforms(camProjMatVal,camViewMatVal,modelMatVal,camNormalMatVal,lightProjMatVal,lightViewMatVal,lightDir,shadowMapParamVal);
    target2.ubuf->updateBuffer(currentFrame);
}


int main(){
    prepareSharedComponents();
    ANTH_LOGI("Preparing Offscreen");
    prepareOffscreen();
    ANTH_LOGI("Preparing Target");
    prepareTarget();
    ANTH_LOGI("Preparing Debug");
    prepareDebug();

    shared.renderer.registerPipelineSubComponents();
    recordCommandsAll();

    //START!
    int currentFrame = 0;
    shared.renderer.setDrawFunction([&](){
        updateOffscrUniform(currentFrame);
        uint32_t imgIdx;
        shared.renderer.drPrepareFrame(currentFrame,&imgIdx);
        shared.renderer.drSubmitBufferPrimaryCall(currentFrame, currentFrame);
        shared.renderer.drPresentFrame(currentFrame,imgIdx);
        currentFrame++;
        currentFrame %= 2;
    });
    ANTH_LOGI("Loop Started");
    shared.renderer.startDrawLoopDemo();
    shared.renderer.finalize();

    return 0;
}