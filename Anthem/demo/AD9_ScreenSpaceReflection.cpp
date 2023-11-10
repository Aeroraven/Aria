#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/external/AnthemGLTFLoader.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/components/camera/AnthemCamera.h"

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::External;
using namespace Anthem::Components::Camera;

struct SharedComponents{
    AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
    AnthemSimpleToyRenderer renderer;
    AnthemConfig config;

    // Uniforms
    using uxProjMat = AnthemUniformMatf<4>;
    using uxModelMat = AnthemUniformMatf<4>;
    using uxViewMat = AnthemUniformMatf<4>;
    using uxNormalMat = AnthemUniformMatf<4>;
    using uxWindowState = AnthemUniformVecf<2>;

    AnthemDescriptorPool* descUniform;
    AnthemUniformBufferImpl<uxProjMat,uxModelMat,uxViewMat,uxNormalMat,uxWindowState>* ubuf;

}shared;


struct LoadedMesh{
    constexpr static int numMeshes = 5;
    using vxPosAttr = AtAttributeVecf<3>;
    using vxNormalAttr = AtAttributeVecf<3>;
    using vxTexPosAttr = AtAttributeVecf<2>;
    using vxMiscAttr = AtAttributeVecf<4>; // (Specular,Textured,0,0)

    AnthemDescriptorPool** descPool;
    AnthemVertexBufferImpl<vxPosAttr,vxNormalAttr,vxTexPosAttr,vxMiscAttr>** vxBuffers;
    AnthemIndexBuffer** ixBuffers;
    AnthemImage** texture;
}meshes;

//Offscreen pass prepares attachments required in deferred SSR pass.
struct OffscreenPass{

    // Render Targets
    AnthemDescriptorPool* descPoolColor; //(R,G,B,X)
    AnthemDescriptorPool* descPoolNormal;
    AnthemDescriptorPool* descPoolPos;
    AnthemDescriptorPool* descPoolSpecular;

    AnthemImage* attachmentColor;
    AnthemImage* attachmentNormal;
    AnthemImage* attachmentPosition;
    AnthemImage* attachmentSpecular;

    AnthemDepthBuffer* depthBuffer;
    AnthemFramebuffer* framebuffer;

    // Pipeline
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;
}offscreenPass;

//SSR pass combines rendered attachments to generate reflections.
struct SSRPass{
    using vxPosAttr = AtAttributeVecf<2>;
    using vxTexAttr = AtAttributeVecf<3>;

    // Render Targets
    AnthemVertexBufferImpl<vxPosAttr,vxTexAttr>* vxBuffer;
    AnthemIndexBuffer* ixBuffer;
    AnthemDepthBuffer* depthBuffer;
    AnthemSwapchainFramebuffer* framebuffer;

    // Pipeline
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;
}ssrPass;

void prepareOffscreenPass(){
    auto& renderer = shared.renderer;

    // Prepare Attachments
    renderer.createDescriptorPool(&offscreenPass.descPoolColor);
    renderer.createDescriptorPool(&offscreenPass.descPoolNormal);
    renderer.createDescriptorPool(&offscreenPass.descPoolPos);
    renderer.createDescriptorPool(&offscreenPass.descPoolSpecular);

    renderer.createColorAttachmentImage(&offscreenPass.attachmentColor,offscreenPass.descPoolColor,0,AT_IF_SRGB_FLOAT32,false);
    renderer.createColorAttachmentImage(&offscreenPass.attachmentNormal,offscreenPass.descPoolNormal,0,AT_IF_SRGB_FLOAT32,false);
    renderer.createColorAttachmentImage(&offscreenPass.attachmentPosition,offscreenPass.descPoolPos,0,AT_IF_SRGB_FLOAT32,false);
    renderer.createColorAttachmentImage(&offscreenPass.attachmentSpecular,offscreenPass.descPoolSpecular,0,AT_IF_SRGB_FLOAT32,false);

    renderer.createDepthBuffer(&offscreenPass.depthBuffer,false);

    // Prepare Pipeline
    AnthenRenderPassSetupOption setupOpt{
        .renderPassUsage = AT_ARPAA_INTERMEDIATE_PASS,
        .msaaType = AT_ARPMT_NO_MSAA,
        .colorAttachmentFormats = { AT_IF_SRGB_FLOAT32,AT_IF_SRGB_FLOAT32,AT_IF_SRGB_FLOAT32,AT_IF_SRGB_FLOAT32 }
    };
    renderer.setupRenderPass(&offscreenPass.pass,&setupOpt,offscreenPass.depthBuffer);
    ANTH_LOGI("Render Pass Created");

    const std::vector<const AnthemImage*> temp = {offscreenPass.attachmentColor,
        offscreenPass.attachmentNormal,offscreenPass.attachmentPosition,offscreenPass.attachmentSpecular};
    renderer.createSimpleFramebuffer(&offscreenPass.framebuffer,&temp,offscreenPass.pass,offscreenPass.depthBuffer);

    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\ssrOffscreen\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\ssrOffscreen\\shader.frag.spv"
    };
    renderer.createShader(&offscreenPass.shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    // Assemble Pipeline
    AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
        .descPool = shared.descUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerDescEntryRegPipeline = {
        .descPool = meshes.descPool[0],
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {samplerDescEntryRegPipeline,uniformBufferDescEntryRegPipeline};

    renderer.createGraphicsPipelineCustomized(&offscreenPass.pipeline,descSetEntriesRegPipeline,offscreenPass.pass,offscreenPass.shader,meshes.vxBuffers[0]);
    ANTH_LOGI("Pipeline Created");
}

void prepareSSRPass(){
    auto& target = ssrPass;
    auto& renderer = shared.renderer;

    //Creating Vertex Buffer & Index Buffer
    renderer.createVertexBuffer(&target.vxBuffer);
    target.vxBuffer->setTotalVertices(4);
    target.vxBuffer->insertData(0,{-1.0f, -1.0f}, {1.0f, 0.0f});
    target.vxBuffer->insertData(1,{1.0f, -1.0f}, {0.0f, 0.0f});
    target.vxBuffer->insertData(2,{1.0f, 1.0f}, {0.0f, 1.0f});
    target.vxBuffer->insertData(3,{-1.0f, 1.0f}, {1.0f, 1.0f});

    renderer.createIndexBuffer(&target.ixBuffer);
    target.ixBuffer->setIndices({0,1,2,2,3,0});

    //Depth Buffer
    renderer.createDepthBuffer(&target.depthBuffer,false);

    //Create Pass
    renderer.setupDemoRenderPass(&target.pass,target.depthBuffer);
    renderer.createSwapchainImageFramebuffers(&target.framebuffer,target.pass,target.depthBuffer);

    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\ssrComposition\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\ssrComposition\\shader.frag.spv"
    };
    renderer.createShader(&target.shader,&shaderFile);
    
    //Create Pipeline
    AnthemDescriptorSetEntry samplerNormal = {
        .descPool = offscreenPass.descPoolNormal,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerPosition = {
        .descPool = offscreenPass.descPoolPos,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerColor = {
        .descPool = offscreenPass.descPoolColor,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerSpecular = {
        .descPool = offscreenPass.descPoolSpecular,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry uniformCam = {
        .descPool = shared.descUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {samplerPosition,samplerColor,samplerNormal,samplerSpecular,uniformCam};
    renderer.createGraphicsPipelineCustomized(&target.pipeline,descSetEntriesRegPipeline,target.pass,target.shader,target.vxBuffer);
    ANTH_LOGI("Done");
}

void recordOffscreenStage(int i){
    auto& renderer = shared.renderer;
    renderer.drStartRenderPass(offscreenPass.pass,(AnthemFramebuffer *)(offscreenPass.framebuffer),i,false);
    renderer.drSetViewportScissor(i);
    renderer.drBindGraphicsPipeline(offscreenPass.pipeline,i);
    for(int j=0;j<meshes.numMeshes;j++){
        AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
            .descPool = shared.descUniform,
            .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
            .inTypeIndex = 0
        };
        AnthemDescriptorSetEntry samplerDescEntryRegPipeline = {
            .descPool = meshes.descPool[j],
            .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
            .inTypeIndex = 0
        };
        std::vector<AnthemDescriptorSetEntry> descSetEntries =  {samplerDescEntryRegPipeline,uniformBufferDescEntryRdw};
        renderer.drBindVertexBuffer(meshes.vxBuffers[j],i);
        renderer.drBindIndexBuffer(meshes.ixBuffers[j],i);
        renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries,offscreenPass.pipeline,i);
        renderer.drDraw(meshes.ixBuffers[j]->getIndexCount(),i);
    }
    renderer.drEndRenderPass(i);
}

void recordSSRStage(int i){
    auto& renderer = shared.renderer;
    auto& target = ssrPass;

    renderer.drStartRenderPass(target.pass,(AnthemFramebuffer *)(target.framebuffer->getFramebufferObject(i)),i,false);
    renderer.drSetViewportScissor(i);
    renderer.drBindGraphicsPipeline(target.pipeline,i);

    AnthemDescriptorSetEntry samplerNormal = {
        .descPool = offscreenPass.descPoolNormal,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerPosition = {
        .descPool = offscreenPass.descPoolPos,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerColor = {
        .descPool = offscreenPass.descPoolColor,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerSpecular = {
        .descPool = offscreenPass.descPoolSpecular,
        .descSetType = AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry uniformCam = {
        .descPool = shared.descUniform,
        .descSetType = AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {samplerPosition,samplerColor,samplerNormal,samplerSpecular,uniformCam};
    renderer.drBindVertexBuffer(target.vxBuffer,i);
    renderer.drBindIndexBuffer(target.ixBuffer,i);
    renderer.drBindDescriptorSetCustomizedGraphics(descSetEntriesRegPipeline,target.pipeline,i);
    renderer.drDraw(target.ixBuffer->getIndexCount(),i);
    renderer.drEndRenderPass(i);
}

void recordCommandsAll(){
    auto& renderer = shared.renderer;
    auto& cfg = shared.config;
    for(int i=0;i<cfg.VKCFG_MAX_IMAGES_IN_FLIGHT;i++){
        renderer.drClearCommands(i);
        renderer.drStartCommandRecording(i);
        recordOffscreenStage(i);
        recordSSRStage(i);
        renderer.drEndCommandRecording(i);
    }
}

void updateOffscrUniform(int currentFrame){
    auto& renderer = shared.renderer;

    int rdWinH,rdWinW;
    renderer.exGetWindowSize(rdWinH,rdWinW);
    auto axis = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*0.3);
    auto localrot = local.clipSubmatrixLeftTop<3,3>();
    
    AtMatf4 camPovProj,camPovView;
    shared.camera.getProjectionMatrix(camPovProj);
    shared.camera.getViewMatrix(camPovView);

    auto viewSpaceRot = camPovView.clipSubmatrixLeftTop<3,3>().multiply(localrot);
    auto modelInvNormal = Math::AnthemLinAlg::inverse3(viewSpaceRot).transpose().padRightBottom<1,1>();
    
    float camProjMatVal[16];
    float camViewMatVal[16];
    float camNormalMatVal[16];
    float modelMatVal[16];
    float windowState[4];

    camPovProj.columnMajorVectorization(camProjMatVal);
    camPovView.columnMajorVectorization(camViewMatVal);
    modelInvNormal.columnMajorVectorization(camNormalMatVal);
    local.columnMajorVectorization(modelMatVal);
    windowState[0] = rdWinW;
    windowState[1] = rdWinH;
    windowState[2] = 0;
    windowState[3] = 0;

    shared.ubuf->specifyUniforms(camProjMatVal,camViewMatVal,modelMatVal,camNormalMatVal,windowState);
    shared.ubuf->updateBuffer(currentFrame);
}

void prepareSharedComponents(){
    // Camera & Renderer Base
    int rdH,rdW;
    shared.renderer.setConfig(&shared.config);
    shared.renderer.initialize();
    shared.renderer.exGetWindowSize(rdH,rdW);
    shared.camera.specifyFrustum((float)AT_PI/2.0f,0.1f,500.0f,1.0f*rdW/rdH);
    shared.camera.specifyPosition(0.0f,70.0f,-120.0f);
    ANTH_LOGI("Intialization Complete");

    // Model Loading
    AnthemGLTFLoader loader;
    AnthemGLTFLoaderParseConfig gltfConfig;
    std::vector<AnthemGLTFLoaderParseResult> gltfResult;
    loader.loadModel("C:\\WR\\Aria\\Anthem\\assets\\gsk\\untitled.gltf");
    loader.parseModel(gltfConfig,gltfResult);
    ANTH_LOGI("Model Loaded");

    // Generate VX/IX Buffer
    auto& renderer = shared.renderer;
    meshes.vxBuffers = new std::remove_pointer_t<decltype(meshes.vxBuffers)>[meshes.numMeshes];
    meshes.ixBuffers = new AnthemIndexBuffer*[meshes.numMeshes];

    // -> Model
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        renderer.createVertexBuffer(&meshes.vxBuffers[chosenMesh]);
        float dfz = 0.1f;
        float dpz = 1.0f;
        uint32_t numVertices = static_cast<uint32_t>(gltfResult.at(chosenMesh).positions.size())/3;
        meshes.vxBuffers[chosenMesh]->setTotalVertices(numVertices);
        for(uint32_t i=0;i<numVertices;i++){
            meshes.vxBuffers[chosenMesh]->insertData(i,
                {gltfResult.at(chosenMesh).positions.at(i*3),gltfResult.at(chosenMesh).positions.at(i*3+1),gltfResult.at(chosenMesh).positions.at(i*3+2)},
                {gltfResult.at(chosenMesh).normals.at(i*3),gltfResult.at(chosenMesh).normals.at(i*3+1),gltfResult.at(chosenMesh).normals.at(i*3+2)},
                {gltfResult.at(chosenMesh).texCoords.at(i*2),gltfResult.at(chosenMesh).texCoords.at(i*2+1)},
                {0.0f,1.0f,0.0f,0.0f}
            );
        }
        ANTH_LOGI("Vertex Buffer Created");
        renderer.createIndexBuffer(&meshes.ixBuffers[chosenMesh]);
        std::vector<uint32_t> indices;
        for(int i=0;i<gltfResult.at(chosenMesh).indices.size();i++){
            indices.push_back(gltfResult.at(chosenMesh).indices.at(i));
        }
        meshes.ixBuffers[chosenMesh]->setIndices(indices);
        ANTH_LOGI("Index Buffer Created");
    }

    // -> Platform
    constexpr static float bw = 120.0;
    constexpr static float bx = 120.0;
    constexpr static float by = 5.0;
    
    renderer.createVertexBuffer(&meshes.vxBuffers[meshes.numMeshes-1]);
    meshes.vxBuffers[meshes.numMeshes-1]->setTotalVertices(4);
    meshes.vxBuffers[meshes.numMeshes-1]->insertData(0,{bx,by,bw},{0.0f,1.0f,0.0f},{0.0f,0.0f},{1.0f,0.0f,0.0f,0.0f});
    meshes.vxBuffers[meshes.numMeshes-1]->insertData(1,{-bx,by,bw},{0.0f,1.0f,0.0f},{0.0f,0.0f},{1.0f,0.0f,0.0f,0.0f});
    meshes.vxBuffers[meshes.numMeshes-1]->insertData(2,{-bx,by,-bw},{0.0f,1.0f,0.0f},{0.0f,0.0f},{1.0f,0.0f,0.0f,0.0f});
    meshes.vxBuffers[meshes.numMeshes-1]->insertData(3,{bx,by,-bw},{0.0f,1.0f,0.0f},{0.0f,0.0f},{1.0f,0.0f,0.0f,0.0f});

    renderer.createIndexBuffer(&meshes.ixBuffers[meshes.numMeshes-1]);
    meshes.ixBuffers[meshes.numMeshes-1]->setIndices(std::vector<uint32_t>({0,1,2,2,3,0}));

    // Prepare Texture
    meshes.descPool = new AnthemDescriptorPool*[gltfResult.size()];
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        renderer.createDescriptorPool(&meshes.descPool[chosenMesh]);
    }

    meshes.texture = new AnthemImage*[meshes.numMeshes];
    for(auto chosenMesh=0;chosenMesh<meshes.numMeshes;chosenMesh++){
        auto imageLoader = new Anthem::External::AnthemImageLoader();
        uint32_t texWidth,texHeight,texChannels;
        uint8_t* texData;
        std::string texPath = "";
        if(chosenMesh<4){
            texPath = gltfResult[chosenMesh].basePath + gltfResult[chosenMesh].pbrBaseColorTexPath;
            if(gltfResult[chosenMesh].pbrBaseColorTexPath==""){
                texPath = "C:\\WR\\Aria\\Anthem\\assets\\cat.jpg";
            }
        }else{
            texPath = "C:\\WR\\Aria\\Anthem\\assets\\cat.jpg";
        }
        imageLoader->loadImage(texPath.c_str(),&texWidth,&texHeight,&texChannels,&texData);
        renderer.createTexture(&meshes.texture[chosenMesh],meshes.descPool[chosenMesh],texData,texWidth,texHeight,texChannels,0,false,false);
        ANTH_LOGI("Texture Created");
    }

    // Prepare Camera Uniform
    renderer.createDescriptorPool(&shared.descUniform);
    renderer.createUniformBuffer(&shared.ubuf,0,shared.descUniform);

    ANTH_LOGI("Shared Components Prepared");
}



int main(){
    prepareSharedComponents();
    prepareOffscreenPass();
    prepareSSRPass();
    ANTH_LOGI("Passes prepared");

    shared.renderer.registerPipelineSubComponents();
    recordCommandsAll();

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