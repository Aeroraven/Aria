#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/external/AnthemImageLoader.h"
#include "../include/external/AnthemGLTFLoader.h"

using namespace Anthem::Core;
using namespace Anthem::External;

int main(){
    ANTH_LOGI("Goodbye World!");
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto app = ANTH_MAKE_SHARED(Anthem::Core::AnthemSimpleToyRenderer)();
    app->setConfig(cfg.get());
    app->initialize();
    ANTH_LOGI("Intialization Complete");

    //Loading Model
    AnthemGLTFLoader loader;
    AnthemGLTFLoaderParseConfig gltfConfig;
    std::vector<AnthemGLTFLoaderParseResult> gltfResult;
    loader.loadModel("/home/funkybirds/Aria/Anthem/assets/gsk/untitled.gltf");
    loader.parseModel(gltfConfig,gltfResult);

    
    //Creating Descriptor Pool
    AnthemDescriptorPool* descPool;
    app->createDescriptorPool(&descPool);


    //Creating Vertex Buffer & Index Buffer
    using vxColorAttr = AnthemVAOAttrDesc<float,3>;
    using vxPosAttr = AnthemVAOAttrDesc<float,3>;
    using vxTexAttr = AnthemVAOAttrDesc<float,2>;

    AnthemVertexBufferImpl<vxPosAttr,vxColorAttr,vxTexAttr>** vxBuffers = new AnthemVertexBufferImpl<vxPosAttr,vxColorAttr,vxTexAttr>*[gltfResult.size()];
    AnthemIndexBuffer** ixBuffers = new AnthemIndexBuffer*[gltfResult.size()];

    for(auto chosenMesh=0;chosenMesh<gltfResult.size();chosenMesh++){
        app->createVertexBuffer(&vxBuffers[chosenMesh]);
        float dfz = 0.1f;
        float dpz = 0.5f;
        int numVertices = gltfResult.at(chosenMesh).positions.size()/3;
        vxBuffers[chosenMesh]->setTotalVertices(numVertices);
        for(int i=0;i<numVertices;i++){
            vxBuffers[chosenMesh]->insertData(i,
                {gltfResult.at(chosenMesh).positions.at(i*3),-gltfResult.at(chosenMesh).positions.at(i*3+1),gltfResult.at(chosenMesh).positions.at(i*3+2)},
                {0.05, 0.0, 0.0},{1.0f, 0.0f});
        }
        ANTH_LOGI("Vertex Buffer Created");

        app->createIndexBuffer(&ixBuffers[chosenMesh]);
        std::vector<uint32_t> indices;
        for(int i=0;i<gltfResult.at(chosenMesh).indices.size();i++){
            indices.push_back(gltfResult.at(chosenMesh).indices.at(i));
        }
        ixBuffers[chosenMesh]->setIndices(indices);
        ANTH_LOGI("Index Buffer Created");
    }

    //Create Uniform Buffer
    AnthemUniformBufferImpl<AnthemUniformVecf<4>,AnthemUniformMatf<4>>* ubuf;
    app->createUniformBuffer(&ubuf,0,descPool);
    float color[4] = {0.1f,0.0f,0.0f,0.0f};
    float matVal[16];
    auto axis = Math::AnthemVector<float,3>({1.0f,0.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,0.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,80.0f,-100.0f});
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransform(0.1f,300.0f,-0.1f,0.1f,0.1f,-0.1f);
    auto lookAt = Math::AnthemLinAlg::modelLookAtTransform(eye,center,up);
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*0.01);
    auto mat = proj.multiply(lookAt.multiply(local));
    mat.columnMajorVectorization(matVal);
    ubuf->specifyUniforms(color,matVal);
    ubuf->updateBuffer(0);
    ubuf->updateBuffer(1);
    
    ANTH_LOGI("Uniform Buffer Created");

    //Create Texture
    auto imageLoader = new Anthem::External::AnthemImageLoader();
    uint32_t texWidth,texHeight,texChannels;
    uint8_t* texData;
    imageLoader->loadImage("/home/funkybirds/Aria/Anthem/assets/cat.jpg",&texWidth,&texHeight,&texChannels,&texData);
    AnthemImage* image;
    app->createTexture(&image,descPool,texData,texWidth,texHeight,texChannels,1);
    ANTH_LOGI("Texture Created");

    //Create Depth Buffer
    auto depthBuffer = new AnthemDepthBuffer();
    app->createDepthBuffer(&depthBuffer);
    ANTH_LOGI("Depth Buffer Created");

    //Create Pass
    AnthemRenderPass* pass;
    app->setupDemoRenderPass(&pass,depthBuffer);
    ANTH_LOGI("Render Pass Created");

    //Create Framebuffer
    AnthemFramebufferList* framebuffer;
    app->createFramebufferList(&framebuffer,pass,depthBuffer);
    ANTH_LOGI("Framebuffer Created", (long long )(framebuffer));

    //Create Shader
    AnthemShaderModule* shader;
    AnthemShaderFilePaths shaderFile = {
        .vertexShader = "/home/funkybirds/Aria/Anthem/shader/default/shader.vert.spv",
        .fragmentShader = "/home/funkybirds/Aria/Anthem/shader/default/shader.frag.spv"
    };
    app->createShader(&shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    //Assemble Pipeline
    AnthemGraphicsPipeline* pipeline;
    app->createPipeline(&pipeline,descPool,pass,shader,vxBuffers[0],ubuf);
    ANTH_LOGI("Pipeline Created");

    //Start Loop
    app->registerPipelineSubComponents();
    int currentFrame = 0;
    app->setDrawFunction([&](){
        auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*0);
        auto mat = proj.multiply(lookAt.multiply(local));
        mat.columnMajorVectorization(matVal);
        ubuf->specifyUniforms(color,matVal);
        ubuf->updateBuffer(currentFrame);

        uint32_t imgIdx;
        app->prepareFrame(currentFrame,&imgIdx);

        app->drStartRenderPass(pass,framebuffer,imgIdx,currentFrame);
        app->drSetViewportScissor(currentFrame);
        app->drBindPipeline(pipeline,currentFrame);
        for(int i=0;i<gltfResult.size();i++){
            app->drBindVertexBuffer(vxBuffers[i],currentFrame);
            app->drBindIndexBuffer(ixBuffers[i],currentFrame);
            app->drBindDescriptorSet(descPool,pipeline,currentFrame);
            app->drDraw(ixBuffers[i]->getIndexCount(),currentFrame);
        }
        app->drEndRenderPass(currentFrame);
        app->drSubmitBuffer(currentFrame);
        app->drPresentFrame(currentFrame,imgIdx);
        
        currentFrame++;
        currentFrame %= 2;
    });
    ANTH_LOGI("Draw Loop Started");
    app->startDrawLoopDemo();
    app->finialize();
    return 0;
}
