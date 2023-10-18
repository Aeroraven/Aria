#include "include/core/renderer/AnthemSimpleToyRenderer.h"
#include "include/core/math/AnthemLinAlg.h"
#include "include/external/AnthemImageLoader.h"
using namespace Anthem::Core;
using namespace Anthem::External;

int main(){
    ANTH_LOGI("Goodbye World!");
    auto cfg = ANTH_MAKE_SHARED(Anthem::Core::AnthemConfig)();
    auto app = ANTH_MAKE_SHARED(Anthem::Core::AnthemSimpleToyRenderer)();
    app->setConfig(cfg.get());
    app->initialize();
    ANTH_LOGI("Intialization Complete");

    //Creating Vertex Buffer
    using vxColorAttr = AnthemVAOAttrDesc<float,3>;
    using vxPosAttr = AnthemVAOAttrDesc<float,2>;
    using vxTexAttr = AnthemVAOAttrDesc<float,2>;
    AnthemVertexBufferImpl<vxPosAttr,vxColorAttr,vxTexAttr>* vxBuffer;
    app->createVertexBuffer(&vxBuffer);
    vxBuffer->setTotalVertices(4);
    vxBuffer->insertData(0,{-0.5f, -0.5f},{0.05, 0.0, 0.0},{1.0f, 0.0f});
    vxBuffer->insertData(1,{0.5f, -0.5f},{0.0, 0.05, 0.0},{0.0f, 0.0f});
    vxBuffer->insertData(2,{0.5f, 0.5f},{0.0, 0.0, 0.05},{0.0f, 1.0f});
    vxBuffer->insertData(3,{-0.5f, 0.5f},{0.05, 0.05, 0.05},{1.0f, 1.0f});
    ANTH_LOGI("Vertex Buffer Created");
    
    //Create Index Buffer
    AnthemIndexBuffer* ixBuffer;
    app->createIndexBuffer(&ixBuffer);
    ixBuffer->setIndices({0,1,2,2,3,0});
    ANTH_LOGI("Index Buffer Created");

    //Create Uniform Buffer
    AnthemUniformBufferImpl<AnthemUniformVecf<4>,AnthemUniformMatf<4>>* ubuf;
    app->createUniformBuffer(&ubuf,0);
    float color[4] = {0.1f,0.0f,0.0f,0.0f};
    float matVal[16];
    auto axis = Math::AnthemVector<float,3>({1.0f,0.0f,0.0f});
    auto center = Math::AnthemVector<float,3>({0.0f,0.0f,0.0f});
    auto eye = Math::AnthemVector<float,3>({0.0f,0.0f,-1.0f});
    auto up = Math::AnthemVector<float,3>({0.0f,1.0f,0.0f});
    auto proj = Math::AnthemLinAlg::spatialPerspectiveTransform(0.1f,100.0f,-0.1f,0.1f,0.1f,-0.1f);
    auto lookAt = Math::AnthemLinAlg::modelLookAtTransform(eye,center,up);
    auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis,(float)glfwGetTime());
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
    app->createTexture(&image,texData,texWidth,texHeight,texChannels,1);
    ANTH_LOGI("Texture Created");

    //Create Pass
    AnthemRenderPass* pass;
    app->setupDemoRenderPass(&pass);
    ANTH_LOGI("Render Pass Created");

    //Create Framebuffer
    AnthemFramebufferList* framebuffer;
    app->createFramebufferList(&framebuffer,pass);
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
    AnthemGraphicsPipeline* piepline;
    app->createPipeline(&piepline,pass,shader,vxBuffer,ubuf);
    ANTH_LOGI("Pipeline Created");

    //Start Draw Loop
    app->registerPipelineSubComponents();
    int currentFrame = 0;
    app->setDrawFunction([&](){
        uint32_t imgIdx;
        app->prepareFrame(currentFrame,&imgIdx);
        app->presentFrameDemo(currentFrame,pass,piepline,framebuffer,imgIdx,vxBuffer,ubuf,ixBuffer);
        currentFrame++;
        currentFrame %= 2;
    });
    ANTH_LOGI("Draw Loop Started");
    app->startDrawLoopDemo();

    app->finialize();
    return 0;
}
