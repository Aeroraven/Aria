#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/external/AnthemGLTFLoader.h"

using namespace Anthem::Core;
using namespace Anthem::External;

int main(){
    ANTH_LOGI("Goodbye World!");
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto renderer = ANTH_MAKE_SHARED(Anthem::Core::AnthemSimpleToyRenderer)();
    renderer->setConfig(cfg.get());
    renderer->initialize();
    ANTH_LOGI("Intialization Complete");

    //Loading Model
    AnthemGLTFLoader loader;
    AnthemGLTFLoaderParseConfig gltfConfig;
    std::vector<AnthemGLTFLoaderParseResult> gltfResult;
    loader.loadModel("C:\\WR\\Aria\\Anthem\\assets\\gsk\\untitled.gltf");
    loader.parseModel(gltfConfig,gltfResult);

    ANTH_LOGI("Model Loaded");
    //Creating Descriptor Pool
    AnthemDescriptorPool** descPool = new AnthemDescriptorPool*[gltfResult.size()];
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        renderer->createDescriptorPool(&descPool[chosenMesh]);
    }


    //Creating Vertex Buffer & Index Buffer
    using vxColorAttr = AnthemVAOAttrDesc<float,3>;
    using vxPosAttr = AnthemVAOAttrDesc<float,3>;
    using vxTexAttr = AnthemVAOAttrDesc<float,2>;

    AnthemVertexBufferImpl<vxPosAttr,vxColorAttr,vxTexAttr>** vxBuffers = new AnthemVertexBufferImpl<vxPosAttr,vxColorAttr,vxTexAttr>*[gltfResult.size()];
    AnthemIndexBuffer** ixBuffers = new AnthemIndexBuffer*[gltfResult.size()];

    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        renderer->createVertexBuffer(&vxBuffers[chosenMesh]);
        float dfz = 0.1f;
        float dpz = 0.5f;
        int numVertices = gltfResult.at(chosenMesh).positions.size()/3;
        vxBuffers[chosenMesh]->setTotalVertices(numVertices);
        for(int i=0;i<numVertices;i++){
            vxBuffers[chosenMesh]->insertData(i,
                {gltfResult.at(chosenMesh).positions.at(i*3),-gltfResult.at(chosenMesh).positions.at(i*3+1),gltfResult.at(chosenMesh).positions.at(i*3+2)},
                {0.05, 0.0, 0.0},
                {gltfResult.at(chosenMesh).texCoords.at(i*2),gltfResult.at(chosenMesh).texCoords.at(i*2+1)});
        }
        ANTH_LOGI("Vertex Buffer Created");

        renderer->createIndexBuffer(&ixBuffers[chosenMesh]);
        std::vector<uint32_t> indices;
        for(int i=0;i<gltfResult.at(chosenMesh).indices.size();i++){
            indices.push_back(gltfResult.at(chosenMesh).indices.at(i));
        }
        ixBuffers[chosenMesh]->setIndices(indices);
        ANTH_LOGI("Index Buffer Created");
    }

    //Create Uniform Buffer
    AnthemUniformBufferImpl<AnthemUniformVecf<4>,AnthemUniformMatf<4>>* ubuf;
    renderer->createUniformBuffer(&ubuf,0,descPool[0]);
    float color[4] = {0.1f,0.0f,0.0f,0.0f};
    float matVal[16];
    auto axis = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,-70.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,-70.0f,-80.0f});
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransform(0.1f,300.0f,-0.1f,0.1f,0.1f,-0.1f);
    auto lookAt = Math::AnthemLinAlg::modelLookAtTransform(eye,center,up);
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*0.00);
    auto mat = proj.multiply(lookAt.multiply(local));
    mat.columnMajorVectorization(matVal);
    ubuf->specifyUniforms(color,matVal);
    ubuf->updateBuffer(0);
    ubuf->updateBuffer(1);
    
    ANTH_LOGI("Uniform Buffer Created");

    //Create Texture
    AnthemImage** image = new AnthemImage*[gltfResult.size()];
    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        auto imageLoader = new Anthem::External::AnthemImageLoader();
        uint32_t texWidth,texHeight,texChannels;
        uint8_t* texData;
        std::string texPath = gltfResult[chosenMesh].basePath + gltfResult[chosenMesh].pbrBaseColorTexPath;
        if(gltfResult[chosenMesh].pbrBaseColorTexPath==""){
            texPath = "C:\\WR\\Aria\\Anthem\\assets\\cat.jpg";
        }
        imageLoader->loadImage(texPath.c_str(),&texWidth,&texHeight,&texChannels,&texData);
        renderer->createTexture(&image[chosenMesh],descPool[chosenMesh],texData,texWidth,texHeight,texChannels,1,true,false);
        image[chosenMesh]->enableMipMapping();
        ANTH_LOGI("Texture Created");
    }

    //Create Depth Buffer
    auto depthBuffer = new AnthemDepthBuffer();
    renderer->createDepthBuffer(&depthBuffer,false);
    ANTH_LOGI("Depth Buffer Created");

    //Create Pass
    AnthemRenderPass* pass;
    renderer->setupDemoRenderPass(&pass,depthBuffer);
    ANTH_LOGI("Render Pass Created");

    //Create Framebuffer
    AnthemSwapchainFramebuffer* framebuffer;
    renderer->createSwapchainImageFramebuffers(&framebuffer,pass,depthBuffer);
    ANTH_LOGI("Framebuffer Created", (long long )(framebuffer));

    //Create Shader
    AnthemShaderModule* shader;
    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\default\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\default\\shader.frag.spv"
    };
    renderer->createShader(&shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    //Assemble Pipeline
    AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
        .descPool = descPool[0],
        .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
        .inTypeIndex = 0
    };
    AnthemDescriptorSetEntry samplerDescEntryRegPipeline = {
        .descPool = descPool[0],
        .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
        .inTypeIndex = 0
    };
    std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = {uniformBufferDescEntryRegPipeline,samplerDescEntryRegPipeline};

    AnthemGraphicsPipeline* pipeline;
    renderer->createPipelineCustomized(&pipeline,descSetEntriesRegPipeline,pass,shader,vxBuffers[0]);
    ANTH_LOGI("Pipeline Created");

    //Start Loop
    renderer->registerPipelineSubComponents();
    

    //Prepare Command
    
    for(int i=0;i<cfg->VKCFG_MAX_IMAGES_IN_FLIGHT;i++){
        renderer->drClearCommands(i);
        renderer->drStartCommandRecording(i);
        renderer->drStartRenderPass(pass,(AnthemFramebuffer *)(framebuffer->getFramebufferObject(i)),i,false);
        renderer->drSetViewportScissor(i);
        renderer->drBindPipeline(pipeline,i);
        for(int j=0;j<gltfResult.size();j++){
            AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
                .descPool = descPool[0],
                .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
                .inTypeIndex = 0
            };
            AnthemDescriptorSetEntry samplerDescEntryRdw = {
                .descPool = descPool[j],
                .descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
                .inTypeIndex = 0
            };
            std::vector<AnthemDescriptorSetEntry> descSetEntries = {uniformBufferDescEntryRdw,samplerDescEntryRdw};
            renderer->drBindVertexBuffer(vxBuffers[j],i);
            renderer->drBindIndexBuffer(ixBuffers[j],i);
            renderer->drBindDescriptorSetCustomized(descSetEntries,pipeline,i);
            renderer->drDraw(ixBuffers[j]->getIndexCount(),i);
        }
        renderer->drEndRenderPass(i);
        renderer->drEndCommandRecording(i);
    }

    //Start DRAW!
    int currentFrame = 0;
    renderer->setDrawFunction([&](){
        int rdWinH,rdWinW;
        renderer->exGetWindowSize(rdWinH,rdWinW);
        auto proj = Math::AnthemLinAlg::spatialPerspectiveTransformWithFovAspect(0.1f,300.0f,(float)M_PI/2.0f,1.0f*rdWinW/rdWinH);
        auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)M_PI*glfwGetTime());
        auto mat = proj.multiply(lookAt.multiply(local));
        mat.columnMajorVectorization(matVal);
        ubuf->specifyUniforms(color,matVal);
        ubuf->updateBuffer(currentFrame);

        uint32_t imgIdx;
        renderer->drPrepareFrame(currentFrame,&imgIdx);

        renderer->drSubmitBuffer(currentFrame);
        renderer->drPresentFrame(currentFrame,imgIdx);
        
        currentFrame++;
        currentFrame %= 2;
    });
    ANTH_LOGI("Loop Started");
    renderer->startDrawLoopDemo();
    renderer->finialize();
    return 0;
}
