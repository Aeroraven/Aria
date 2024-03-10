#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/external/AnthemImageLoader.h"
using namespace Anthem::Core;
using namespace Anthem::External;

// AD1 Triangles
// =======================
// Hello World Program

int main(){
    ANTH_LOGI("Goodbye World!");
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto renderer = ANTH_MAKE_SHARED(Anthem::Core::AnthemSimpleToyRenderer)();
    renderer->setConfig(cfg.get());
    renderer->initialize();
    ANTH_LOGI("Intialization Complete");

    //Creating Descriptor Pool
    AnthemDescriptorPool* descPool;
    renderer->createDescriptorPool(&descPool);

    //Creating Vertex Buffer
    using vxColorAttr = AnthemVAOAttrDesc<float,3>;
    using vxPosAttr = AnthemVAOAttrDesc<float,3>;
    using vxTexAttr = AnthemVAOAttrDesc<float,2>;
    AnthemVertexBufferImpl<vxPosAttr,vxColorAttr,vxTexAttr>* vxBuffer;
    renderer->createVertexBuffer(&vxBuffer);
    vxBuffer->setTotalVertices(8);
    float dfz = 0.1f;
    float dpz = 0.5f;
    for(int T=0;T<8;T+=4){
        vxBuffer->insertData(0+T,{-0.5f+T*dfz, -0.5f+T*dfz, 1.0f+T*dpz},{0.05f, 0.0f, 0.0f},{1.0f, 0.0f});
        vxBuffer->insertData(1+T,{0.5f+T*dfz, -0.5f+T*dfz, 1.0f+T*dpz},{0.0f, 0.05f, 0.0f},{0.0f, 0.0f});
        vxBuffer->insertData(2+T,{0.5f+T*dfz, 0.5f+T*dfz, 1.0f+T*dpz},{0.0f, 0.0f, 0.05f},{0.0f, 1.0f});
        vxBuffer->insertData(3+T,{-0.5f+T*dfz, 0.5f+T*dfz, 1.0f+T*dpz},{0.05f, 0.05f, 0.05f},{1.0f, 1.0f});
    }
    ANTH_LOGI("Vertex Buffer Created");
    
    //Create Index Buffer
    AnthemIndexBuffer* ixBuffer;
    renderer->createIndexBuffer(&ixBuffer);
    ixBuffer->setIndices({0,1,2,2,3,0,4,5,6,6,7,4});
    ANTH_LOGI("Index Buffer Created");

    //Create Uniform Buffer
    AnthemUniformBufferImpl<AnthemUniformVecf<4>,AnthemUniformMatf<4>>* ubuf;
    renderer->createUniformBuffer(&ubuf,0,descPool);
    float color[4] = {0.1f,0.0f,0.0f,0.0f};
    float matVal[16];
    auto axis = Math::AnthemVector<float,3>({1.0f,0.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,0.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,0.0f,-0.5f});
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransform(0.1f,100.0f,-0.1f,0.1f,0.1f,-0.1f);
    auto lookAt = Math::AnthemLinAlg::lookAtTransform(eye,center,up);
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
    renderer->createTexture(&image,descPool,texData,texWidth,texHeight,texChannels,1,true,false);
    ANTH_LOGI("Texture Created");

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
        .vertexShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\default\\shader.vert.spv",
        .fragmentShader = "C:\\WR\\Aria\\Anthem\\shader\\glsl\\default\\shader.frag.spv"
    };
    renderer->createShader(&shader,&shaderFile);
    ANTH_LOGI("Shader Created");

    //Assemble Pipeline
    AnthemGraphicsPipeline* pipeline;
    renderer->createGraphicsPipeline(&pipeline,descPool,pass,shader,vxBuffer,ubuf);
    ANTH_LOGI("Pipeline Created");

    //Start Loop
    renderer->registerPipelineSubComponents();
    int currentFrame = 0;
    renderer->setDrawFunction([&](){
        auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime());
        auto mat = proj.multiply(lookAt.multiply(local));
        mat.columnMajorVectorization(matVal);
        ubuf->specifyUniforms(color,matVal);
        ubuf->updateBuffer(currentFrame);

        uint32_t imgIdx;
        renderer->drPrepareFrame(currentFrame,&imgIdx);
        renderer->drClearCommands(currentFrame);
        renderer->drStartCommandRecording(currentFrame);
        renderer->drStartRenderPass(pass,(AnthemFramebuffer *)(framebuffer->getFramebufferObject(currentFrame)),currentFrame,false);
        renderer->drSetViewportScissor(currentFrame);
        renderer->drBindGraphicsPipeline(pipeline,currentFrame);
        renderer->drBindVertexBuffer(vxBuffer,currentFrame);
        renderer->drBindIndexBuffer(ixBuffer,currentFrame);
        renderer->drBindDescriptorSet(descPool,pipeline,currentFrame,currentFrame);
        renderer->drDraw(ixBuffer->getIndexCount(),currentFrame);
        renderer->drEndRenderPass(currentFrame);
        renderer->drEndCommandRecording(currentFrame);
        renderer->drSubmitBufferPrimaryCall(currentFrame, currentFrame);
        renderer->drPresentFrame(currentFrame,imgIdx);
        
        currentFrame++;
        currentFrame %= 2;
    });
    ANTH_LOGI("Draw Loop Started");
    renderer->startDrawLoopDemo();
    renderer->finalize();
    return 0;
}
