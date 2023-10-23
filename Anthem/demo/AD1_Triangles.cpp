#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/external/AnthemImageLoader.h"
using namespace Anthem::Core;
using namespace Anthem::External;

int main(){
    ANTH_LOGI("Goodbye World!");
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto app = ANTH_MAKE_SHARED(Anthem::Core::AnthemSimpleToyRenderer)();
    app->setConfig(cfg.get());
    app->initialize();
    ANTH_LOGI("Intialization Complete");

    //Creating Descriptor Pool
    AnthemDescriptorPool* descPool;
    app->createDescriptorPool(&descPool);

    //Creating Vertex Buffer
    using vxColorAttr = AnthemVAOAttrDesc<float,3>;
    using vxPosAttr = AnthemVAOAttrDesc<float,3>;
    using vxTexAttr = AnthemVAOAttrDesc<float,2>;
    AnthemVertexBufferImpl<vxPosAttr,vxColorAttr,vxTexAttr>* vxBuffer;
    app->createVertexBuffer(&vxBuffer);
    vxBuffer->setTotalVertices(8);
    float dfz = 0.1f;
    float dpz = 0.5f;
    for(int T=0;T<8;T+=4){
        vxBuffer->insertData(0+T,{-0.5f+T*dfz, -0.5f+T*dfz, 1.0f+T*dpz},{0.05, 0.0, 0.0},{1.0f, 0.0f});
        vxBuffer->insertData(1+T,{0.5f+T*dfz, -0.5f+T*dfz, 1.0f+T*dpz},{0.0, 0.05, 0.0},{0.0f, 0.0f});
        vxBuffer->insertData(2+T,{0.5f+T*dfz, 0.5f+T*dfz, 1.0f+T*dpz},{0.0, 0.0, 0.05},{0.0f, 1.0f});
        vxBuffer->insertData(3+T,{-0.5f+T*dfz, 0.5f+T*dfz, 1.0f+T*dpz},{0.05, 0.05, 0.05},{1.0f, 1.0f});
    }
    ANTH_LOGI("Vertex Buffer Created");
    
    //Create Index Buffer
    AnthemIndexBuffer* ixBuffer;
    app->createIndexBuffer(&ixBuffer);
    ixBuffer->setIndices({0,1,2,2,3,0,4,5,6,6,7,4});
    ANTH_LOGI("Index Buffer Created");

    //Create Uniform Buffer
    AnthemUniformBufferImpl<AnthemUniformVecf<4>,AnthemUniformMatf<4>>* ubuf;
    app->createUniformBuffer(&ubuf,0,descPool);
    float color[4] = {0.1f,0.0f,0.0f,0.0f};
    float matVal[16];
    auto axis = Math::AnthemVector<float,3>({1.0f,0.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,0.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,0.0f,-0.5f});
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransform(0.1f,100.0f,-0.1f,0.1f,0.1f,-0.1f);
    auto lookAt = Math::AnthemLinAlg::modelLookAtTransform(eye,center,up);
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime()*200);
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
    imageLoader->loadImage("C:\\WR\\Aria\\Anthem\\assets\\cat.jpg",&texWidth,&texHeight,&texChannels,&texData);
    AnthemImage* image;
    app->createTexture(&image,descPool,texData,texWidth,texHeight,texChannels,1,true);
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
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\default\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\default\\shader.frag.spv"
    };
    app->createShader(&shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    //Assemble Pipeline
    AnthemGraphicsPipeline* pipeline;
    app->createPipeline(&pipeline,descPool,pass,shader,vxBuffer,ubuf);
    ANTH_LOGI("Pipeline Created");

    //Start Loop
    app->registerPipelineSubComponents();
    int currentFrame = 0;
    app->setDrawFunction([&](){
        auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime());
        auto mat = proj.multiply(lookAt.multiply(local));
        mat.columnMajorVectorization(matVal);
        ubuf->specifyUniforms(color,matVal);
        ubuf->updateBuffer(currentFrame);

        uint32_t imgIdx;
        app->prepareFrame(currentFrame,&imgIdx);
        app->drStartRenderPass(pass,framebuffer,imgIdx,currentFrame);
        app->drSetViewportScissor(currentFrame);
        app->drBindPipeline(pipeline,currentFrame);
        app->drBindVertexBuffer(vxBuffer,currentFrame);
        app->drBindIndexBuffer(ixBuffer,currentFrame);
        app->drBindDescriptorSet(descPool,pipeline,currentFrame);
        app->drDraw(ixBuffer->getIndexCount(),currentFrame);
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
