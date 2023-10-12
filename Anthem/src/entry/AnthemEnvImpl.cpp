#include "../../include/entry/AnthemEnvImpl.h"

namespace Anthem::Entry{
    AnthemEnvImpl::AnthemEnvImpl(ANTH_SHARED_PTR(AnthemConfig) cfg){
        this->cfg = cfg;
        this->instance = ANTH_MAKE_SHARED(AnthemInstance)();
        this->instance->specifyConfig(cfg.get());
        this->valLayer = ANTH_MAKE_SHARED(AnthemValLayer)(cfg);
    }
    bool AnthemEnvImpl::createWindow(){
        this->instance->createWindow();
        this->windowSurface = ANTH_MAKE_SHARED(AnthemWindowSurface)((GLFWwindow*)(this->instance->getWindow()));
        return true;
    }
    void AnthemEnvImpl::drawLoop(){
        std::function<void()> drawFunc = std::bind(&AnthemEnvImpl::drawFrame,this);
        this->instance->startDrawLoop(drawFunc);
        this->logicalDevice->waitForIdle();
    }
    void AnthemEnvImpl::createInstance(){
        this->instance->createInstance();
        if(cfg->VKCFG_ENABLE_VALIDATION_LAYERS){
            auto w = this->instance->getCreateInfo()->pNext;
            valLayer->fillingPointerData(this->instance->getCreateInfoPNext());
        }
        this->instance->specifyResizeHandler([&](int w,int h){
            ANTH_LOGI("Window resized, w=",w," h=",h);
            this->resizeStatusFlag = true;
        });
    }
    void AnthemEnvImpl::destroySwapChain(){
         //Destroy Framebuffer
        framebufferList->destroyFramebuffers();

        //Destroy Swap Chain Stuffs
        swapChain->destroySwapChainImageViews(this->logicalDevice.get());
        swapChain->destroySwapChain(this->logicalDevice.get());
    }
    void AnthemEnvImpl::recreateSwapChain(){
        ANTH_LOGW("Recreating Swapchain");
        this->instance->waitForFramebufferReady();
        this->logicalDevice->waitForIdle();

        this->destroySwapChain();
        this->swapChain->specifySwapChainDetails(this->phyDevice.get(),this->instance->getWindow());
        this->swapChain->createSwapChain(this->logicalDevice.get(),this->phyDevice.get());
        this->swapChain->retrieveSwapChainImages(this->logicalDevice.get());
        this->swapChain->createSwapChainImageViews(this->logicalDevice.get());
        this->viewport->prepareViewportState();
        this->initFramebuffer();
    }
    void AnthemEnvImpl::destroyEnv(){
        ANTH_LOGI("Destroying environment");
        
        this->destroySwapChain();

        //Destroy Sync Objects
        mainLoopSyncer->destroySyncObjects();

        //Destroy Command Objects
        commandManager->destroyCommandPool();
       
        //Destroy Graphics Pipeline
        graphicsPipeline->destroyPipeline();
        graphicsPipeline->destroyPipelineLayout();
        shader->destroyShaderModules(this->logicalDevice.get());

        //Destroy Render Pawss
        renderPass->destroyRenderPass();

        //Destroy Device & Layers
        logicalDevice->destroyLogicalDevice(this->instance->getInstance());
        valLayer->destroyDebugMsgLayer(this->instance->getInstance());
        windowSurface->destroyWindowSurface(this->instance->getInstance());

        //Destroy Instance
        this->instance->destroyInstance();
        this->instance->destroyWindow();
    }
    void AnthemEnvImpl::init(){
        //Startup
        this->createWindow();
        this->initSwapChain();
        valLayer->createDebugMsgLayerInfo();
        this->createInstance();

        valLayer->createDebugMsgLayer(this->instance->getInstance());
        windowSurface->createWindowSurface(this->instance->getInstance());

        //Selecting Physical Device
        phyDeviceSelector = ANTH_MAKE_SHARED(AnthemPhyDeviceSelector)(windowSurface,swapChain);
        phyDeviceSelector->selectPhyDevice(this->instance->getInstance(),this->windowSurface);
        phyDevice = ANTH_MAKE_SHARED(AnthemPhyDevice)();
        phyDeviceSelector->getPhyDevice(this->phyDevice.get());

        //Creating Logical Device
        logicalDeviceSelector = ANTH_MAKE_SHARED(AnthemLogicalDeviceSelector)(this->phyDevice.get());
        logicalDeviceSelector->createLogicalDevice();
        logicalDevice = ANTH_MAKE_SHARED(AnthemLogicalDevice)();
        logicalDeviceSelector->getLogicalDevice(logicalDevice.get());

        //Prepare Swap Chain
        swapChain->specifySwapChainDetails(this->phyDevice.get(),this->instance->getWindow());
        swapChain->createSwapChain(this->logicalDevice.get(),this->phyDevice.get());
        swapChain->retrieveSwapChainImages(this->logicalDevice.get());
        swapChain->createSwapChainImageViews(this->logicalDevice.get());

        //Create Render Pass
        this->initRenderPass();

        //Create Graphics Pipeline
        this->loadShader();
        this->initGraphicsPipeline();

        //Create Drawing Objects
        this->initFramebuffer();
        this->initCommandManager();
        this->initSyncObjects();

        ANTH_LOGI("Environment initialized");
    }
    void AnthemEnvImpl::run(){
        this->init();
        this->drawLoop();
        this->destroyEnv();
    } 
    void AnthemEnvImpl::initSwapChain(){
        this->swapChain = ANTH_MAKE_SHARED(AnthemSwapChain)(this->windowSurface);
    }
    void AnthemEnvImpl::loadShader(){
        this->shader = ANTH_MAKE_SHARED(AnthemShaderModule)();
        AnthemShaderFilePaths shaderPath = {
            .vertexShader = "/home/funkybirds/Aria/Anthem/shader/default/shader.vert.spv",
            .fragmentShader = "/home/funkybirds/Aria/Anthem/shader/default/shader.frag.spv"
        };
        this->shader->createShaderModules(this->logicalDevice.get(),&shaderPath);
    }
    void AnthemEnvImpl::initGraphicsPipeline(){
        //Setup Viewport
        this->viewport = ANTH_MAKE_SHARED(AnthemViewport)();
        this->viewport->specifyLogicalDevice(this->logicalDevice.get());
        this->viewport->specifySwapChain(this->swapChain.get());
        this->viewport->prepareViewportState();

        //Prepare Graphics Pipeline
        this->graphicsPipeline = ANTH_MAKE_SHARED(AnthemGraphicsPipeline)();
        this->graphicsPipeline->specifyLogicalDevice(this->logicalDevice.get());
        this->graphicsPipeline->specifyViewport(this->viewport.get());
        this->graphicsPipeline->specifyRenderPass(this->renderPass.get());
        this->graphicsPipeline->specifyShaderModule(this->shader.get());

        this->graphicsPipeline->preparePreqPipelineCreateInfo();
        this->graphicsPipeline->createPipelineLayout();
        this->graphicsPipeline->createPipeline();
    }
    void AnthemEnvImpl::initRenderPass(){
        this->renderPass = ANTH_MAKE_SHARED(AnthemRenderPass)();
        this->renderPass->specifyLogicalDevice(this->logicalDevice.get());
        this->renderPass->specifySwapChain(this->swapChain.get());
        this->renderPass->createDemoRenderPass();
    }
    void AnthemEnvImpl::initFramebuffer(){
        this->framebufferList = ANTH_MAKE_SHARED(AnthemFramebufferList)();
        this->framebufferList->specifyLogicalDevice(this->logicalDevice.get());
        this->framebufferList->createFramebuffersFromSwapChain(this->swapChain.get(),this->renderPass.get());
    }
    void AnthemEnvImpl::initCommandManager(){
        this->commandManager = ANTH_MAKE_SHARED(AnthemCommandManager)();
        this->commandManager->specifyLogicalDevice(this->logicalDevice.get());
        this->commandManager->specifyConfig(this->cfg.get());
        this->commandManager->specifyPhyDevice(this->phyDevice.get());
        this->commandManager->specifySwapChain(this->swapChain.get());
        this->commandManager->createCommandPool();
        this->commandManager->createCommandBuffer();
    }
    void AnthemEnvImpl::initSyncObjects(){
        this->mainLoopSyncer = ANTH_MAKE_SHARED(AnthemMainLoopSyncer)();
        this->mainLoopSyncer->specifyLogicalDevice(this->logicalDevice.get());
        this->mainLoopSyncer->specifyConfig(this->cfg.get());
        this->mainLoopSyncer->specifySwapChain(this->swapChain.get());
        this->mainLoopSyncer->createSyncObjects();
    }
    void AnthemEnvImpl::drawFrame(){
        ANTH_LOGV("Start Drawing");
        this->mainLoopSyncer->waitForPrevFrame(currentFrame);
        auto imageIdx = this->mainLoopSyncer->acquireNextFrame(currentFrame,[&](){
            this->recreateSwapChain();
            this->resizeStatusFlag = false;
        });
        if(imageIdx == UINT32_MAX){
            ANTH_LOGW("Suboptimal Swap Chain, drawFrame interrupted");
            return;
        }

        ANTH_LOGV("Acquired Image Idx=",imageIdx);
        this->commandManager->resetCommandBuffer(currentFrame);

        ANTH_LOGV("Wait For Command");
        AnthemCommandManagerRenderPassStartInfo beginInfo = {
            .renderPass = this->renderPass.get(),
            .framebufferList = this->framebufferList.get(),
            .framebufferIdx = imageIdx,
            .clearValue = {{{0.0f,0.0f,0.0f,1.0f}}},
        };
        this->commandManager->startCommandRecording(currentFrame);
        this->commandManager->startRenderPass(&beginInfo,currentFrame);
        this->commandManager->demoDrawCommand(this->graphicsPipeline.get(),this->viewport.get(),currentFrame);
        this->commandManager->endRenderPass(currentFrame);
        this->commandManager->endCommandRecording(currentFrame);

        this->mainLoopSyncer->submitCommandBuffer(this->commandManager->getCommandBuffer(currentFrame),currentFrame);
        
        //Presentation
        auto presentRes = this->mainLoopSyncer->presentFrame(imageIdx,currentFrame,[&](){
            this->recreateSwapChain();
            this->resizeStatusFlag = false;
        });
        if(this->resizeStatusFlag){
            this->recreateSwapChain();
            this->resizeStatusFlag = false;
        }
        if(!presentRes){
            ANTH_LOGW("Suboptimal Swap Chain, drawFrame interrupted");
        }
        currentFrame = (currentFrame+1)%this->cfg->VKCFG_MAX_IMAGES_IN_FLIGHT;
    }
    
}



