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

struct StagePass{
    using vxPosAttr = AtAttributeVecf<3>;
    using vxNormalAttr = AtAttributeVecf<3>;

    using uxProjMat = AnthemUniformMatf<4>;
    using uxModelMat = AnthemUniformMatf<4>;
    using uxViewMat = AnthemUniformMatf<4>;
    using uxNormalMat = AnthemUniformMatf<4>;

    AnthemDescriptorPool* descPoolUniform;

    AnthemVertexBufferImpl<vxPosAttr,vxNormalAttr>** vxBuffers;
    AnthemIndexBuffer** ixBuffers;
    AnthemUniformBufferImpl<uxProjMat,uxViewMat,uxModelMat,uxNormalMat>* ubuf;

    AnthemDepthBuffer* depthBuffer;
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;

    AnthemSwapchainFramebuffer* framebuffer;
    
    int numMeshes = 4;
};

void preparStage(StagePass& target, AnthemSimpleToyRenderer& renderer){
    // Loading Model
    AnthemGLTFLoader loader;
    AnthemGLTFLoaderParseConfig gltfConfig;
    std::vector<AnthemGLTFLoaderParseResult> gltfResult;
    loader.loadModel("C:\\WR\\Aria\\Anthem\\assets\\gsk\\untitled.gltf");
    loader.parseModel(gltfConfig,gltfResult);
    ANTH_LOGI("Model Loaded");

    //Creating Descriptor Pool
    renderer.createDescriptorPool(&target.descPoolUniform);

    //Creating Vertex Buffer & Index Buffer
    target.vxBuffers = new std::remove_pointer_t<decltype(target.vxBuffers)>[gltfResult.size()];
    target.ixBuffers = new AnthemIndexBuffer*[gltfResult.size()];
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        renderer.createVertexBuffer(&target.vxBuffers[chosenMesh]);
        float dfz = 0.1f;
        float dpz = 1.0f;
        int numVertices = gltfResult.at(chosenMesh).positions.size()/3;
        target.vxBuffers[chosenMesh]->setTotalVertices(numVertices);
        for(int i=0;i<numVertices;i++){
            target.vxBuffers[chosenMesh]->insertData(i,
                {gltfResult.at(chosenMesh).positions.at(i*3),-gltfResult.at(chosenMesh).positions.at(i*3+1),gltfResult.at(chosenMesh).positions.at(i*3+2)},
                {gltfResult.at(chosenMesh).normals.at(i*3),-gltfResult.at(chosenMesh).normals.at(i*3+1),gltfResult.at(chosenMesh).normals.at(i*3+2)}
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

    //Create Uniform Buffer
    renderer.createUniformBuffer(&target.ubuf,0,target.descPoolUniform);

    //Create Depth Buffer
    renderer.createDepthBuffer(&target.depthBuffer,false);

    //Create Pass
    AnthenRenderPassSetupOption setupOpt{
        .renderPassUsage = AT_ARPAA_FINAL_PASS,
        .msaaType = AT_ARPMT_NO_MSAA,
        .colorAttachmentFormats = { std::nullopt }
    };
    renderer.setupRenderPass(&target.pass,&setupOpt,target.depthBuffer);
    renderer.createSwapchainImageFramebuffers(&target.framebuffer,target.pass,target.depthBuffer);
    ANTH_LOGI("Render Pass Created");

    //Create Shader
    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\geometryNormal\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\geometryNormal\\shader.frag.spv",
        .geometryShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\geometryNormal\\shader.geom.spv",
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

    renderer.createPipelineCustomized(&target.pipeline,descSetEntriesRegPipeline,target.pass,target.shader,target.vxBuffers[0]);
    ANTH_LOGI("Pipeline Created");
}

void recordCommandsStage(AnthemConfig* cfg,AnthemSimpleToyRenderer& renderer, StagePass& target,int i){
    //Prepare Command
    renderer.drStartRenderPass(target.pass,(AnthemFramebuffer *)(target.framebuffer->getFramebufferObject(i)),i,false);
    renderer.drSetViewportScissor(i);
    renderer.drBindPipeline(target.pipeline,i);
    for(int j=0;j<target.numMeshes;j++){
        AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
            .descPool = target.descPoolUniform,
            .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
            .inTypeIndex = 0
        };
        std::vector<AnthemDescriptorSetEntry> descSetEntries = {uniformBufferDescEntryRdw};
        renderer.drBindVertexBuffer(target.vxBuffers[j],i);
        renderer.drBindIndexBuffer(target.ixBuffers[j],i);
        renderer.drBindDescriptorSetCustomized(descSetEntries,target.pipeline,i);
        renderer.drDraw(target.ixBuffers[j]->getIndexCount(),i);
    }
    renderer.drEndRenderPass(i);
}
void recordCommandsAll(AnthemConfig* cfg,AnthemSimpleToyRenderer& renderer, StagePass& target){
    for(int i=0;i<cfg->VKCFG_MAX_IMAGES_IN_FLIGHT;i++){
        renderer.drClearCommands(i);
        renderer.drStartCommandRecording(i);
        recordCommandsStage(cfg, renderer, target,i);
        renderer.drEndCommandRecording(i);
    }
}

void updateOffscrUniform(StagePass& target ,AnthemSimpleToyRenderer& renderer,int currentFrame){
    int rdWinH,rdWinW;
    renderer.exGetWindowSize(rdWinH,rdWinW);
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransformWithFovAspect(0.1f,300.0f,(float)M_PI/2.0f,1.0f*rdWinW/rdWinH);
    auto axis = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,-70.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,-70.0f,-80.0f});
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto lookAt = Math::AnthemLinAlg::lookAtTransform(eye,center,up);
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*0.0);
    
    auto modelview = lookAt.multiply(local);

    auto modelrot = lookAt.clipSubmatrixLeftTop<3,3>();
    auto localrot = local.clipSubmatrixLeftTop<3,3>();
    auto modelviewrot = modelrot.multiply(localrot);
    auto normalMat = Math::AnthemLinAlg::inverse3(modelviewrot).transpose().padRightBottom<1,1>();

    float projMatVal[16];
    float viewMatVal[16];
    float modelMatVal[16];
    float normalMatVal[16];

    proj.columnMajorVectorization(projMatVal);
    lookAt.columnMajorVectorization(viewMatVal);
    local.columnMajorVectorization(modelMatVal);
    normalMat.columnMajorVectorization(normalMatVal);

    target.ubuf->specifyUniforms(projMatVal,viewMatVal,modelMatVal,normalMatVal);
    target.ubuf->updateBuffer(currentFrame);
}

int main(){
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto renderer = ANTH_MAKE_SHARED(Anthem::Core::AnthemSimpleToyRenderer)();
    renderer->setConfig(cfg.get());
    renderer->initialize();
    ANTH_LOGI("Intialization Complete");

    StagePass target;
    preparStage(target,*renderer.get());

    ANTH_LOGI("Start Reg");
    renderer->registerPipelineSubComponents();

    ANTH_LOGI("Start Rec Commands");
    recordCommandsAll(cfg.get(),*renderer.get(),target);

    //START!
    int currentFrame = 0;
    renderer->setDrawFunction([&](){
        updateOffscrUniform(target,*renderer.get(),currentFrame);
        uint32_t imgIdx;
        renderer->drPrepareFrame(currentFrame,&imgIdx);
        renderer->drSubmitBuffer(currentFrame);
        renderer->drPresentFrame(currentFrame,imgIdx);
        currentFrame++;
        currentFrame %= 2;
    });
    ANTH_LOGI("Loop Started");
    renderer->startDrawLoopDemo();
    renderer->finalize();

    return 0;
}