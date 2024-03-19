#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/external/AnthemGLTFLoader.h"

using namespace Anthem::Core;
using namespace Anthem::External;


struct OffscreenPass{
    using vxColorAttr = AnthemVAOAttrDesc<float,3>;
    using vxPosAttr = AnthemVAOAttrDesc<float,3>;
    using vxTexAttr = AnthemVAOAttrDesc<float,2>;

    AnthemDescriptorPool** descPool;
    AnthemDescriptorPool* descPoolColorAtt;
    AnthemVertexBufferImpl<vxPosAttr,vxColorAttr,vxTexAttr>** vxBuffers;
    AnthemIndexBuffer** ixBuffers;
    AnthemUniformBufferImpl<AnthemUniformVecf<4>,AnthemUniformMatf<4>>* ubuf;
    AnthemImage** image;
    AnthemDepthBuffer* depthBuffer;
    AnthemRenderPass* pass;
    AnthemGraphicsPipeline* pipeline;
    AnthemShaderModule* shader;
    AnthemSwapchainFramebuffer* framebuffer;
    AnthemImage* colorAttachment;

    int numMeshes = 4;
};

void prepareOffscreen(OffscreenPass& offscreen,AnthemSimpleToyRenderer& renderer){
    //Loading Model
    AnthemGLTFLoader loader;
    AnthemGLTFLoaderParseConfig gltfConfig;
    std::vector<AnthemGLTFLoaderParseResult> gltfResult;
    loader.loadModel("C:\\WR\\Aria\\Anthem\\assets\\gsk\\untitled.gltf");
    loader.parseModel(gltfConfig,gltfResult);

    ANTH_LOGI("Model Loaded");
    //Creating Descriptor Pool
    renderer.createDescriptorPool(&offscreen.descPoolColorAtt);

    offscreen.descPool = new AnthemDescriptorPool*[gltfResult.size()];
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        renderer.createDescriptorPool(&offscreen.descPool[chosenMesh]);
    }

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
                {0.05f, 0.0f, 0.0f},
                {gltfResult.at(chosenMesh).texCoords.at(i*2),gltfResult.at(chosenMesh).texCoords.at(i*2+1)});
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
    renderer.createUniformBuffer(&offscreen.ubuf,0,offscreen.descPool[0]);
    float color[4] = {0.1f,0.0f,0.0f,0.0f};
    float matVal[16];
    auto axis = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,-70.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,-70.0f,-80.0f});
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransform(0.1f,300.0f,-0.1f,0.1f,0.1f,-0.1f);
    auto lookAt = Math::AnthemLinAlg::lookAtTransform(eye,center,up);
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*0.00);
    auto mat = proj.multiply(lookAt.multiply(local));
    mat.columnMajorVectorization(matVal);
    offscreen.ubuf->specifyUniforms(color,matVal);
    offscreen.ubuf->updateBuffer(0);
    offscreen.ubuf->updateBuffer(1);
    ANTH_LOGI("Uniform Buffer Created");

    //Create Texture
    offscreen.image = new AnthemImage*[gltfResult.size()];
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        auto imageLoader = new Anthem::External::AnthemImageLoader();
        uint32_t texWidth,texHeight,texChannels;
        uint8_t* texData;
        std::string texPath = gltfResult[chosenMesh].basePath + gltfResult[chosenMesh].pbrBaseColorTexPath;
        if(gltfResult[chosenMesh].pbrBaseColorTexPath==""){
            texPath = "C:\\WR\\Aria\\Anthem\\assets\\cat.jpg";
        }
        imageLoader->loadImage(texPath.c_str(),&texWidth,&texHeight,&texChannels,&texData);
        renderer.createTexture(&offscreen.image[chosenMesh],offscreen.descPool[chosenMesh],texData,texWidth,texHeight,texChannels,1,true,false);
        offscreen.image[chosenMesh]->enableMipMapping();
    }
    ANTH_LOGI("Texture Created");

    //Create Depth Buffer
    renderer.createDepthBuffer(&offscreen.depthBuffer,true);
    ANTH_LOGI("Depth Buffer Created");

    //Create Attachment
    renderer.createColorAttachmentImage(&offscreen.colorAttachment,offscreen.descPoolColorAtt,0,AT_IF_SBGR_UINT8,true);

    //Create Pass
    AnthemRenderPassSetupOption setupOpt{
        .renderPassUsage = AT_ARPAA_FINAL_PASS,
        .msaaType = AT_ARPMT_MSAA,
        .msaaColorAttachment = offscreen.colorAttachment,
        .msaaSamples = (VkSampleCountFlagBits)(offscreen.colorAttachment->getSampleCount())
    };
    renderer.setupRenderPass(&offscreen.pass,&setupOpt,offscreen.depthBuffer);
    ANTH_LOGI("Render Pass Created");

    //Create Framebuffer
    renderer.createSwapchainImageFramebuffers(&offscreen.framebuffer,offscreen.pass,offscreen.depthBuffer);
    ANTH_LOGI("Framebuffer Created", (long long )(offscreen.framebuffer));

    //Create Shader
    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\default\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\default\\shader.frag.spv"
    };
    renderer.createShader(&offscreen.shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    //Assemble Pipeline
    AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
        .descPool = offscreen.descPool[0],
        .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerDescEntryRegPipeline = {
        .descPool = offscreen.descPool[0],
        .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {uniformBufferDescEntryRegPipeline,samplerDescEntryRegPipeline};

    renderer.createGraphicsPipelineCustomized(&offscreen.pipeline,descSetEntriesRegPipeline, {}, offscreen.pass,offscreen.shader,offscreen.vxBuffers[0], nullptr);
    ANTH_LOGI("Pipeline Created");
}




void recordCommandsOffscreen(AnthemConfig* cfg,AnthemSimpleToyRenderer& renderer, OffscreenPass& offscreen,int i){
    //Prepare Command
    renderer.drStartRenderPass(offscreen.pass,(AnthemFramebuffer *)(offscreen.framebuffer->getFramebufferObject(i)),i,true);
    renderer.drSetViewportScissorFromSwapchain(i);
    renderer.drBindGraphicsPipeline(offscreen.pipeline,i);
    for(int j=0;j<offscreen.numMeshes;j++){
        AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
            .descPool = offscreen.descPool[0],
            .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
            .inTypeIndex = 0
        };
        AnthemDescriptorSetEntry samplerDescEntryRdw = {
            .descPool = offscreen.descPool[j],
            .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
            .inTypeIndex = 0
        };
        std::vector<AnthemDescriptorSetEntry> descSetEntries = {uniformBufferDescEntryRdw,samplerDescEntryRdw};
        renderer.drBindVertexBuffer(offscreen.vxBuffers[j],i);
        renderer.drBindIndexBuffer(offscreen.ixBuffers[j],i);
        renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries,offscreen.pipeline,i);
        renderer.drDraw(offscreen.ixBuffers[j]->getIndexCount(),i);
    }
    renderer.drEndRenderPass(i);
}

void updateOffscrUniform(OffscreenPass& offscr,AnthemSimpleToyRenderer& renderer,int currentFrame){
    int rdWinH,rdWinW;
    renderer.exGetWindowSize(rdWinH,rdWinW);
    float color[4] = {0.1f,0.0f,0.0f,0.0f};
    float matVal[16];
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto axis = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,-70.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,-70.0f,-80.0f});
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransformWithFovAspect(0.1f,300.0f,(float)AT_PI/2.0f,1.0f*rdWinW/rdWinH);
    auto lookAt = Math::AnthemLinAlg::lookAtTransform(eye,center,up);
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)AT_PI*glfwGetTime()*0.01);
    auto mat = proj.multiply(lookAt.multiply(local));
    mat.columnMajorVectorization(matVal);
    offscr.ubuf->specifyUniforms(color,matVal);
    offscr.ubuf->updateBuffer(currentFrame);
}

int main(){
    auto cfg = ANTH_MAKE_UNIQUE(Anthem::Core::AnthemConfig)();
    auto renderer = ANTH_MAKE_UNIQUE(Anthem::Core::AnthemSimpleToyRenderer)();
    renderer->setConfig(cfg.get());
    renderer->initialize();
    ANTH_LOGI("Intialization Complete");

    OffscreenPass offscr;

    //Prepare Components
    prepareOffscreen(offscr,*renderer.get());
    
    //Start Loop
    ANTH_LOGI("Start Reg");
    renderer->registerPipelineSubComponents();

    ANTH_LOGI("Start Rec Commands");
    for(int i=0;i<cfg->vkcfgMaxImagesInFlight;i++){
        renderer->drStartCommandRecording(i);
        recordCommandsOffscreen(cfg.get(),*renderer.get(),offscr,i);
        renderer->drEndCommandRecording(i);
    }

    //Start DRAW!
    int currentFrame = 0;
    renderer->setDrawFunction([&](){
        updateOffscrUniform(offscr,*renderer.get(),currentFrame);
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
    return 0;
}