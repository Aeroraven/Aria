#include "../../../include/core/renderer/AnthemSimpleToyRenderer.h"

namespace Anthem::Core{
    bool AnthemSimpleToyRenderer::setConfig(const AnthemConfig* config){
        this->config = config;
        return true;
    }
    bool AnthemSimpleToyRenderer::finialize(){
        this->setupState = false;
        ANTH_LOGI("Finalizing");
        this->logicalDevice->waitForIdle();
        this->destroySwapChain();

        for(auto& p:this->descriptorPools){
            p->destroyDescriptorPool();
            p->destroyLayoutBinding();
            delete p;
        }

        for(const auto& p:this->uniformBuffers){
            p->destroyBuffers();
            delete p;
        }
        for(const auto& p:this->textures){
            p->destroyImage();
            delete p;
        }
        for(const auto& p:this->vertexBuffers){
            p->destroyBuffer();
            delete p;
        }
        for(const auto& p:this->indexBuffers){
            p->destroyBuffer();
            delete p;
        }

        this->mainLoopSyncer->destroySyncObjects();
        this->commandBuffers->destroyCommandPool();

        for(const auto& p:this->graphicsPipelines){
            p->destroyPipeline();
            p->destroyPipelineLayout();
            delete p;
        }
        for(const auto& p:this->shaders){
            p->destroyShaderModules(this->logicalDevice.get());
            delete p;
        }
        for(const auto& p:this->renderPasses){
            p->destroyRenderPass();
            delete p;
        }
        logicalDevice->destroyLogicalDevice(this->instance->getInstance());
        validationLayer->destroyDebugMsgLayer(this->instance->getInstance());
        windowSurface->destroyWindowSurface(this->instance->getInstance());

        //Destroy Instance
        this->instance->destroyInstance();
        this->instance->destroyWindow();

        return true;
    }
    bool AnthemSimpleToyRenderer::initialize(){
        //Step1. Create Instance & Validation Layer
        this->instance = ANTH_MAKE_UNIQUE(AnthemInstance)();
        this->instance->specifyConfig(this->config);
        this->validationLayer = ANTH_MAKE_UNIQUE(AnthemValLayer)((AnthemConfig*)this->config);
        
        this->instance->createWindow();
        this->windowSurface = ANTH_MAKE_UNIQUE(AnthemWindowSurface)((GLFWwindow*)this->instance->getWindow());
        this->validationLayer->createDebugMsgLayerInfo();
        this->instance->createInstance();

        this->instance->specifyResizeHandler([&](int w,int h){
            this->resizeRefreshState = true;
            this->windowHeight = h;
            this->windowWidth = w;
        });

        if(this->config->VKCFG_ENABLE_VALIDATION_LAYERS){
            this->validationLayer->fillingPointerData(this->instance->getCreateInfoPNext());
            this->validationLayer->createDebugMsgLayer(this->instance->getInstance());
        }
        this->windowHeight = this->config->APP_RESLOUTION_H;
        this->windowWidth = this->config->APP_RESLOUTION_W;

        //Step2. Create Window Surface
        this->windowSurface->createWindowSurface(this->instance->getInstance());

        //Step3. Create Swap Chain & Physical Device
        this->swapChain = ANTH_MAKE_UNIQUE(AnthemSwapChain)(this->windowSurface.get());
        this->phyDeviceSelector = ANTH_MAKE_UNIQUE(AnthemPhyDeviceSelector)(this->windowSurface.get(),this->swapChain.get());
        this->phyDeviceSelector->selectPhyDevice(this->instance->getInstance(),this->windowSurface.get());
        this->phyDevice = ANTH_MAKE_UNIQUE(AnthemPhyDevice)();
        this->phyDeviceSelector->getPhyDevice(this->phyDevice.get());

        //Step4. Create Logical Device
        this->logicalDeviceSelector = ANTH_MAKE_UNIQUE(AnthemLogicalDeviceSelector)(this->phyDevice.get());
        this->logicalDeviceSelector->createLogicalDevice();
        this->logicalDevice = ANTH_MAKE_UNIQUE(AnthemLogicalDevice)();
        this->logicalDeviceSelector->getLogicalDevice(this->logicalDevice.get());

        //Step5. Prepare Swapchain
        this->swapChain->specifySwapChainDetails(this->phyDevice.get(),this->instance->getWindow());
        this->swapChain->createSwapChain(this->logicalDevice.get(),this->phyDevice.get());
        this->swapChain->retrieveSwapChainImages(this->logicalDevice.get());
        this->swapChain->createSwapChainImageViews(this->logicalDevice.get());

        //Step6. Prepare Command Buffer
        this->commandBuffers = ANTH_MAKE_UNIQUE(AnthemCommandBuffers)();
        this->commandBuffers->specifyConfig(this->config);
        this->commandBuffers->specifyLogicalDevice(this->logicalDevice.get());
        this->commandBuffers->specifyPhyDevice(this->phyDevice.get());
        this->commandBuffers->specifySwapChain(this->swapChain.get());
        this->commandBuffers->createCommandPool();

        //Step7. Create Descriptor Pool
        // Removed

        //Step8. Create Main Loop Syncer
        this->mainLoopSyncer = ANTH_MAKE_UNIQUE(AnthemMainLoopSyncer)();
        this->mainLoopSyncer->specifyLogicalDevice(this->logicalDevice.get());
        this->mainLoopSyncer->specifyConfig(this->config);
        this->mainLoopSyncer->specifySwapChain(this->swapChain.get());
        this->mainLoopSyncer->createSyncObjects();

        //Step9. Create Viewport
        this->viewport = ANTH_MAKE_UNIQUE(AnthemViewport)();
        this->viewport->specifyLogicalDevice(this->logicalDevice.get());
        this->viewport->specifySwapChain(this->swapChain.get());
        this->viewport->prepareViewportState();

        //Step10. Create Draw Helper (Need to Remove)
        this->drawingCommandHelper = ANTH_MAKE_UNIQUE(AnthemDrawingCommandHelper)();
        this->drawingCommandHelper->specifyConfig(this->config);
        this->drawingCommandHelper->specifyCommandBuffers(this->commandBuffers.get());
        this->drawingCommandHelper->specifySwapChain(this->swapChain.get());
        this->drawingCommandHelper->initializeHelper();
        
        this->setupState = true;
        return true;
    }

    bool AnthemSimpleToyRenderer::createDescriptorPool(AnthemDescriptorPool** pDescPool){
        auto descriptorPool = new AnthemDescriptorPool();
        descriptorPool->specifyLogicalDevice(this->logicalDevice.get());
        descriptorPool->createDescriptorPool(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,&(this->uniformDescPoolIdx));
        descriptorPool->createDescriptorPool(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ,&(this->imageDescPoolIdx));
        *pDescPool = descriptorPool;
        this->descriptorPools.push_back(descriptorPool);
        return true;
    }
    bool AnthemSimpleToyRenderer::setupRenderPass(AnthemRenderPass** pRenderPass, AnthenRenderPassSetupOption* setupOption, AnthemDepthBuffer* depthBuffer){
        auto newRenderPass = new AnthemRenderPass();
        newRenderPass->specifyLogicalDevice(this->logicalDevice.get());
        newRenderPass->specifySwapChain(this->swapChain.get());
        newRenderPass->setDepthBuffer(depthBuffer);
        newRenderPass->createRenderPass(*setupOption);
        this->renderPasses.push_back(newRenderPass);
        *pRenderPass = newRenderPass;
        return true;
    }
    bool AnthemSimpleToyRenderer::setupDemoRenderPass(AnthemRenderPass** pRenderPass,AnthemDepthBuffer* depthBuffer){
        auto newRenderPass = new AnthemRenderPass();
        newRenderPass->specifyLogicalDevice(this->logicalDevice.get());
        newRenderPass->specifySwapChain(this->swapChain.get());
        newRenderPass->setDepthBuffer(depthBuffer);
        newRenderPass->createDemoRenderPass();
        this->renderPasses.push_back(newRenderPass);
        *pRenderPass = newRenderPass;
        return true;
    }

    bool AnthemSimpleToyRenderer::createTexture(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth,
        uint32_t texHeight, uint32_t texChannel, uint32_t bindLoc,bool generateMipmap){
            
        //Allocate Image
        auto textureImage = new AnthemImage();
        textureImage->specifyLogicalDevice(this->logicalDevice.get());
        textureImage->specifyPhyDevice(this->phyDevice.get());
        textureImage->specifyCommandBuffers(this->commandBuffers.get());
        textureImage->loadImageData(texData,texWidth,texHeight,texChannel);
        textureImage->specifyUsage(AnthemImageUsage::AT_IU_TEXTURE2D);
        if(generateMipmap){
            textureImage->enableMipMapping();
        }
        textureImage->prepareImage();

        //Allocate Descriptor Set For Sampler
        ANTH_LOGI("In addsampler");
        descPool->addSampler(textureImage,bindLoc,this->imageDescPoolIdx);
        *pImage = textureImage;
        this->textures.push_back(textureImage);
        return true;
    }

    bool AnthemSimpleToyRenderer::createColorAttachmentImage(AnthemImage** pImage,AnthemDescriptorPool* descPool, uint32_t bindLoc){
        auto textureImage = new AnthemImage();
        textureImage->specifyLogicalDevice(this->logicalDevice.get());
        textureImage->specifyPhyDevice(this->phyDevice.get());
        textureImage->specifyCommandBuffers(this->commandBuffers.get());
        textureImage->setImageSize(this->swapChain->getSwapChainExtentWidth(),this->swapChain->getSwapChainExtentHeight());
        textureImage->specifyUsage(AT_IU_COLOR_ATTACHMENT);
        ANTH_LOGI("Creating Color Attachment");
        textureImage->prepareImage();

        //Allocate Descriptor Set For Sampler
        descPool->addSampler(textureImage,bindLoc,this->imageDescPoolIdx);
        *pImage = textureImage;
        this->textures.push_back(textureImage);
        return true;
    }

    bool AnthemSimpleToyRenderer::registerPipelineSubComponents(){
        //Prepare Attr & Ind Buffers
        for(const auto& p:vertexBuffers){
            p->createBuffer();
        }

        ANTH_LOGI("XXXXXXXXXXXXXXXXXxx Vertex done");
        for(const auto& p:indexBuffers){
            p->createBuffer();
        }
        ANTH_LOGI("XXXXXXXXXXXXXXXXXxx Buffers done");
        //Prepare Descriptor Sets
        for(const auto& p:descriptorPools){
            ANTH_LOGI("XXXXXXXXXXXXXXXXXxx Registering:",(long long)(&p));
            p->createDescriptorSet(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT);
        }
        return true;
    }

    bool AnthemSimpleToyRenderer::createIndexBuffer(AnthemIndexBuffer** pIndexBuffer){
        auto idxBuffer = new AnthemIndexBuffer();
        idxBuffer->specifyLogicalDevice(this->logicalDevice.get());
        idxBuffer->specifyPhyDevice(this->phyDevice.get());
        idxBuffer->specifyCommandBuffers(this->commandBuffers.get());
        *pIndexBuffer = idxBuffer;
        this->indexBuffers.push_back(idxBuffer);
        return true;
    }
    
    bool AnthemSimpleToyRenderer::createShader(AnthemShaderModule** pShaderModule,AnthemShaderFilePaths* filename){
        auto shaderModule = new AnthemShaderModule();
        shaderModule->createShaderModules(this->logicalDevice.get(),filename);
        *pShaderModule = shaderModule;
        this->shaders.push_back(shaderModule);
        return true;
    }
    bool AnthemSimpleToyRenderer::createPipelineCustomized(AnthemGraphicsPipeline** pPipeline,std::vector<AnthemDescriptorSetEntry> descSetEntries,AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule,AnthemVertexBuffer* vertexBuffer){
        auto graphicsPipeline = new AnthemGraphicsPipeline();
        graphicsPipeline->specifyLogicalDevice(this->logicalDevice.get());
        graphicsPipeline->specifyViewport(this->viewport.get());
        graphicsPipeline->specifyRenderPass(renderPass);
        graphicsPipeline->specifyShaderModule(shaderModule);
        graphicsPipeline->specifyVertexBuffer(vertexBuffer);
        graphicsPipeline->specifyUniformBuffer(nullptr); 
        ANTH_TODO("Remove nullptr");
        graphicsPipeline->specifyDescriptor(descSetEntries.at(0).descPool);

        graphicsPipeline->preparePreqPipelineCreateInfo();
        graphicsPipeline->createPipelineLayoutCustomized(descSetEntries);
        graphicsPipeline->createPipeline();
        this->graphicsPipelines.push_back(graphicsPipeline);
        *pPipeline = graphicsPipeline;
        return true;
    }
    bool AnthemSimpleToyRenderer::createPipeline(AnthemGraphicsPipeline** pPipeline,  AnthemDescriptorPool* descPool, AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule,AnthemVertexBuffer* vertexBuffer,AnthemUniformBuffer* uniformBuffer){
        auto graphicsPipeline = new AnthemGraphicsPipeline();
        graphicsPipeline->specifyLogicalDevice(this->logicalDevice.get());
        graphicsPipeline->specifyViewport(this->viewport.get());
        graphicsPipeline->specifyRenderPass(renderPass);
        graphicsPipeline->specifyShaderModule(shaderModule);
        graphicsPipeline->specifyVertexBuffer(vertexBuffer);
        graphicsPipeline->specifyUniformBuffer(uniformBuffer);
        graphicsPipeline->specifyDescriptor(descPool);

        graphicsPipeline->preparePreqPipelineCreateInfo();
        graphicsPipeline->createPipelineLayout();
        graphicsPipeline->createPipeline();
        this->graphicsPipelines.push_back(graphicsPipeline);
        *pPipeline = graphicsPipeline;
        return true;
    }
    bool AnthemSimpleToyRenderer::createSimpleFramebuffer(AnthemFramebuffer** pFramebuffer, const AnthemImage* colorAttachment, const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer){
        ANTH_TODO("Forced type cast");
        auto fb = new AnthemFramebuffer();
        fb->specifyLogicalDevice(this->logicalDevice.get());
        fb->setDepthBuffer((AnthemDepthBuffer*)depthBuffer);
        fb->createFromColorAttachment(colorAttachment,renderPass);
        *pFramebuffer = fb;
        this->simpleFramebuffers.push_back(fb);
        return true;
    }

    bool AnthemSimpleToyRenderer::createSwapchainImageFramebuffers(AnthemSwapchainFramebuffer** pFramebufferList,const AnthemRenderPass* renderPass,const AnthemDepthBuffer* depthBuffer){
        ANTH_TODO("Forced type cast");
        auto framebufferList = new AnthemSwapchainFramebuffer();
        framebufferList->specifyLogicalDevice(this->logicalDevice.get());
        framebufferList->setDepthBuffer((AnthemDepthBuffer*)depthBuffer);
        framebufferList->createFramebuffersFromSwapChain(this->swapChain.get(),renderPass);
        *pFramebufferList = framebufferList;
        this->framebufferListObjs.push_back(framebufferList);
        return true;
    }

    bool AnthemSimpleToyRenderer::destroySwapChain(){
        for(const auto& p:this->simpleFramebuffers){
            p->destroyFramebuffers();
        }
        for(const auto& p:this->framebufferListObjs){
            p->destroyFramebuffers();
        }
        for(const auto& p:this->depthBuffers){
            p->destroyDepthBuffer();
        }
        this->swapChain->destroySwapChainImageViews(this->logicalDevice.get());
        this->swapChain->destroySwapChain(this->logicalDevice.get());
        return true;
    }
    bool AnthemSimpleToyRenderer::recreateSwapChain(){
        ANTH_LOGW("Recreating Swapchain");
        this->instance->waitForFramebufferReady();
        this->logicalDevice->waitForIdle();
        this->destroySwapChain();
        this->swapChain->specifySwapChainDetails(this->phyDevice.get(),this->instance->getWindow());
        this->swapChain->createSwapChain(this->logicalDevice.get(),this->phyDevice.get());
        this->swapChain->retrieveSwapChainImages(this->logicalDevice.get());
        this->swapChain->createSwapChainImageViews(this->logicalDevice.get());
        this->viewport->prepareViewportState();
        for(const auto& p:this->depthBuffers){
            p->createDepthBuffer();
        }
        for(const auto& p:this->framebufferListObjs){
            p->createFramebuffersFromSwapChain(this->swapChain.get(),this->renderPasses.at(0));
        }
        for(auto& p:this->simpleFramebuffers){
            p->recreateFramebuffer();
        }
        ANTH_LOGW("Recreating Swapchain, done");
        return true;
    }
    bool AnthemSimpleToyRenderer::drClearCommands(uint32_t frameIdx){
        this->commandBuffers->resetCommandBuffer(frameIdx);
        return true;
    }

    bool AnthemSimpleToyRenderer::prepareFrame(uint32_t currentFrame, uint32_t* avaImageIdx){
        ANTH_LOGV("Preparing Frame");
        this->mainLoopSyncer->waitForPrevFrame(currentFrame);
        auto imageIdx = this->mainLoopSyncer->acquireNextFrame(currentFrame,[&](){
            this->recreateSwapChain();
            this->resizeRefreshState = false;
        });
        if(imageIdx == UINT32_MAX){
            ANTH_LOGW("Suboptimal Swap Chain, drawFrame interrupted");
            return false;
        }
        *avaImageIdx = imageIdx;
        ANTH_LOGV("Presenting Frame, Done");
        return true;
    }
    bool AnthemSimpleToyRenderer::drEndCommandRecording(uint32_t frameIdx){
        this->drawingCommandHelper->endCommandBuffer(frameIdx);
        return true;
    }
    bool AnthemSimpleToyRenderer::drStartCommandRecording(uint32_t frameIdx){
        ANTH_LOGI("Record starts");
        this->drawingCommandHelper->startCommandBuffer(frameIdx);
        ANTH_LOGI("Record starts, done");
        return true;
    }
    bool AnthemSimpleToyRenderer::drStartRenderPass(AnthemRenderPass* renderPass,AnthemFramebuffer* framebuffer ,uint32_t frameIdx){
        AnthemCommandManagerRenderPassStartInfo beginInfo = {
            .renderPass = renderPass,
            .framebufferList = framebuffer,
            .framebufferIdx = 0,
            .clearValue = {{{0.0f,0.0f,0.0f,1.0f}}},
            .depthClearValue = {{1.0f, 0}}
        };
        this->drawingCommandHelper->startRenderPass(&beginInfo,frameIdx);
        return true;
    }
    bool AnthemSimpleToyRenderer::drEndRenderPass(uint32_t frameIdx){
        this->drawingCommandHelper->endRenderPass(frameIdx);
        return true;
    }
    bool AnthemSimpleToyRenderer::drSubmitBuffer(uint32_t frameIdx){
        this->mainLoopSyncer->submitCommandBuffer(this->commandBuffers->getCommandBuffer(frameIdx),frameIdx);
        return true;
    }
    bool AnthemSimpleToyRenderer::drPresentFrame(uint32_t frameIdx, uint32_t avaImageIdx){
        //Presentation
        auto presentRes = this->mainLoopSyncer->presentFrame(avaImageIdx,frameIdx,[&](){
            this->recreateSwapChain();
            this->resizeRefreshState = false;
        });
        if(this->resizeRefreshState){
            this->recreateSwapChain();
            this->resizeRefreshState = false;
        }
        if(!presentRes){
            ANTH_LOGW("Suboptimal Swap Chain, drawFrame interrupted");
        }
        ANTH_LOGV("Present Frame, DONE");
        return true;
    }

    bool AnthemSimpleToyRenderer::drSetViewportScissor(uint32_t frameIdx){
        vkCmdSetViewport(*this->commandBuffers->getCommandBuffer(frameIdx),0,1,viewport->getViewport());
        vkCmdSetScissor(*this->commandBuffers->getCommandBuffer(frameIdx),0,1,viewport->getScissor());
        return true;
    }

    bool AnthemSimpleToyRenderer::drBindPipeline(AnthemGraphicsPipeline* pipeline,uint32_t frameIdx){
        vkCmdBindPipeline(*this->commandBuffers->getCommandBuffer(frameIdx),VK_PIPELINE_BIND_POINT_GRAPHICS,*(pipeline->getPipeline()));
        return true;
    }

    bool AnthemSimpleToyRenderer::drBindVertexBuffer(AnthemVertexBuffer* vertexBuffer,uint32_t frameIdx){
        vkCmdBindVertexBuffers(*this->commandBuffers->getCommandBuffer(frameIdx),0,1,(vertexBuffer->getDestBufferObject()),emptyOffsetPlaceholder);
        return true;
    }
    bool AnthemSimpleToyRenderer::drBindIndexBuffer(AnthemIndexBuffer* indexBuffer,uint32_t frameIdx){
        vkCmdBindIndexBuffer(*this->commandBuffers->getCommandBuffer(frameIdx), *(indexBuffer->getDestBufferObject()), 0, VK_INDEX_TYPE_UINT32);
        return true;
    }
    bool AnthemSimpleToyRenderer::drBindDescriptorSetCustomized(std::vector<AnthemDescriptorSetEntry> descSetEntries, AnthemGraphicsPipeline* pipeline, uint32_t frameIdx){
        std::vector<VkDescriptorSet>* descSets = new std::vector<VkDescriptorSet>();
        for(const auto& p:descSetEntries){
            if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER){
                p.descPool->appendDescriptorSetSampler(p.inTypeIndex,descSets);
            }else if(p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER){
                p.descPool->appendDescriptorSetUniform(p.inTypeIndex,descSets);
            }else{
                ANTH_LOGE("Unknown Descriptor Set Type");
            }
        }
        vkCmdBindDescriptorSets(*this->commandBuffers->getCommandBuffer(frameIdx), VK_PIPELINE_BIND_POINT_GRAPHICS, *(pipeline->getPipelineLayout()), 0,
            descSets->size(),descSets->data() , 0, nullptr);
        delete descSets;
        return true;
    }
    bool AnthemSimpleToyRenderer::drBindDescriptorSet(AnthemDescriptorPool* descPool, AnthemGraphicsPipeline* pipeline, uint32_t frameIdx){
        std::vector<VkDescriptorSet>* descSets = new std::vector<VkDescriptorSet>();
        descPool->getAllDescriptorSets(frameIdx,descSets);
        vkCmdBindDescriptorSets(*this->commandBuffers->getCommandBuffer(frameIdx), VK_PIPELINE_BIND_POINT_GRAPHICS, *(pipeline->getPipelineLayout()), 0,
            descSets->size(),descSets->data() , 0, nullptr);
        delete descSets;
        return true;
    }
    bool AnthemSimpleToyRenderer::drDraw(uint32_t vertices,uint32_t frameIdx){
        vkCmdDrawIndexed(*this->commandBuffers->getCommandBuffer(frameIdx), vertices, 1, 0, 0, 0);
        return true;
    }

    bool AnthemSimpleToyRenderer::presentFrameDemo(uint32_t currentFrame, AnthemRenderPass* renderPass, 
    AnthemGraphicsPipeline* pipeline, AnthemSwapchainFramebuffer* framebuffer,uint32_t avaImageIdx,
    AnthemVertexBuffer* vbuf, AnthemUniformBuffer* ubuf,AnthemIndexBuffer* ibuf,AnthemDescriptorPool* descPool){
        ANTH_DEPRECATED_MSG;
        return false;
    }

    bool AnthemSimpleToyRenderer::startDrawLoopDemo(){
        this->instance->startDrawLoop(this->drawLoopHandler);
        this->logicalDevice->waitForIdle();
        return true;
    }

    bool AnthemSimpleToyRenderer::setDrawFunction(std::function<void()> drawLoopHandler){
        this->drawLoopHandler = drawLoopHandler;
        return true;
    }
    bool AnthemSimpleToyRenderer::createDepthBuffer(AnthemDepthBuffer** pDepthBuffer){
        auto depthBuffer = new AnthemDepthBuffer();
        depthBuffer->specifyLogicalDevice(this->logicalDevice.get());
        depthBuffer->specifyPhyDevice(this->phyDevice.get());
        depthBuffer->specifyCommandBuffers(this->commandBuffers.get());
        depthBuffer->specifySwapChain(this->swapChain.get());
        depthBuffer->createDepthBuffer();
        this->depthBuffers.push_back(depthBuffer);
        *pDepthBuffer = depthBuffer;
        return true;
    }
    bool AnthemSimpleToyRenderer::exGetWindowSize(int& height,int& width){
        height = this->windowHeight;
        width = this->windowWidth;
        return true;
    }
    AnthemSimpleToyRenderer::~AnthemSimpleToyRenderer(){
        if(this->setupState){
            this->finialize();
        }
    }
}