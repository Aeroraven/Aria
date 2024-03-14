#include "../../../include/core/renderer/AnthemSimpleToyRenderer.h"

namespace Anthem::Core{
    bool AnthemSimpleToyRenderer::setConfig(const AnthemConfig* config){
        this->config = config;
        return true;
    }
    bool AnthemSimpleToyRenderer::finalize(){
        this->setupState = false;
        ANTH_LOGI("Finalizing");

        this->logicalDevice->waitForIdle();
        this->destroySwapChain();
        ANTH_LOGV("Swapchain Destroyed");

        for(auto& p:this->descriptorPools){
            p->destroyDescriptorPool();
            p->destroyLayoutBinding();
            delete p;
        }
        ANTH_LOGV("Desc Pools Destroyed");

        for(auto& p:this->uniformBuffers){
            p->destroyBuffers();
            ANTH_LOGV("Preparing to delete uniform buffer");
            delete p;
        }
        ANTH_LOGV("Uniform Buffers Destroyed");

        for (auto& p : this->extFences) {
            p->destroyFence();
        }
        ANTH_LOGV("Fences Destroyed");

        for (auto& p : this->extSemaphores) {
            p->destroySemaphore();
        }
        ANTH_LOGV("Semaphores Destroyed");

        for (auto& p : this->ssboBuffers) {
            p->destroyStagingBuffer();
            p->destroySSBO();
        }
        ANTH_LOGV("SSBO Destroyed");

        for(auto& p:this->textures){
            p->destroyImage();
            delete p;
        }
        for (auto& p : this->texCubes) {
            p->destroyImage();
            delete p;
        }
        ANTH_LOGV("Textures Destroyed");

        for(auto& p:this->vertexBuffers){
            p->destroyBuffer();
            delete p;
        }
        ANTH_LOGV("Vertex Buffers Destroyed");

        for(auto& p:this->indexBuffers){
            p->destroyBuffer();
            delete p;
        }
        ANTH_LOGV("Index Buffers Destroyed");
        for (auto& p : this->indirectDrawBuffers) {
            p->destroyBuffer();
            delete p;
        }
        ANTH_LOGV("Indirect Buffers Destroyed");


        this->mainLoopSyncer->destroySyncObjects();
        this->commandBuffers->destroyCommandPool();
        ANTH_LOGV("Synchronization Objects & Command Pools Destroyed");

        for(auto& p:this->graphicsPipelines){
            p->destroyPipeline();
            p->destroyPipelineLayout();
            delete p;
        }
        for (auto& p : this->computePipelines) {
            p->destroyPipeline();
            p->destroyPipelineLayout();
            delete p;
        }

        ANTH_LOGV("Pipelines Destroyed");

        for(auto& p:this->shaders){
            p->destroyShaderModules(this->logicalDevice.get());
            delete p;
        }
        ANTH_LOGV("Shaders Destroyed");

        for(auto& p:this->renderPasses){
            p->destroyRenderPass();
            delete p;
        }
        ANTH_LOGV("Render Passes Destroyed");

        logicalDevice->destroyLogicalDevice(this->instance->getInstance());
        validationLayer->destroyDebugMsgLayer(this->instance->getInstance());
        windowSurface->destroyWindowSurface(this->instance->getInstance());

        //Destroy Instance
        this->instance->destroyInstance();
        this->instance->destroyWindow();

        ANTH_LOGV("Finalization done");

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
        this->logicalDevice->preparePFNs();

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
        descriptorPool->createDescriptorPool(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &(this->ssboDescPoolIdx));
        descriptorPool->createDescriptorPool(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &(this->storageImgDescPoolIdx));
        *pDescPool = descriptorPool;
        this->descriptorPools.push_back(descriptorPool);
        return true;
    }
    bool AnthemSimpleToyRenderer::setupRenderPass(AnthemRenderPass** pRenderPass, AnthemRenderPassSetupOption* setupOption, AnthemDepthBuffer* depthBuffer){
        auto newRenderPass = new AnthemRenderPass();
        newRenderPass->specifyLogicalDevice(this->logicalDevice.get());
        newRenderPass->specifySwapChain(this->swapChain.get());
        newRenderPass->setDepthBuffer(depthBuffer);
        
        newRenderPass->createRenderPass(*setupOption);
        this->renderPasses.push_back(newRenderPass);
        *pRenderPass = newRenderPass;
        return true;
    }
    bool AnthemSimpleToyRenderer::setupDemoRenderPass(AnthemRenderPass** pRenderPass,AnthemDepthBuffer* depthBuffer,bool retain){
        auto newRenderPass = new AnthemRenderPass();
        newRenderPass->specifyLogicalDevice(this->logicalDevice.get());
        newRenderPass->specifySwapChain(this->swapChain.get());
        newRenderPass->setDepthBuffer(depthBuffer);
        newRenderPass->createDemoRenderPass(retain);
        this->renderPasses.push_back(newRenderPass);
        *pRenderPass = newRenderPass;
        return true;
    }

    bool AnthemSimpleToyRenderer::createTexture(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth,
        uint32_t texHeight, uint32_t texChannel, uint32_t bindLoc,bool generateMipmap2D, bool enableMsaa, AnthemImageFormat imageFmt,
        uint32_t descId, bool ignoreDescPool, AnthemImageUsage usage){

        //Allocate Image
        auto textureImage = new AnthemImage();
        textureImage->specifyLogicalDevice(this->logicalDevice.get());
        textureImage->specifyPhyDevice(this->phyDevice.get());
        textureImage->specifyCommandBuffers(this->commandBuffers.get());
        textureImage->loadImageData(texData, texWidth, texHeight, texChannel);
        textureImage->specifyUsage(usage);
        if(generateMipmap2D){
            textureImage->enableMipMapping();
        }
        if(enableMsaa){
            textureImage->enableMsaa();
        }
        textureImage->setImageFormat(imageFmt);
        textureImage->prepareImage();

        //Allocate Descriptor Set For Sampler
        if (descId == -1) {
            ANTH_LOGW("Descriptor pool index not specified for Tex2D, using the default value", this->imageDescPoolIdx);
            descId = this->imageDescPoolIdx;
        }
        if(!ignoreDescPool) descPool->addSampler(textureImage,bindLoc, descId);
        *pImage = textureImage;
        this->textures.push_back(textureImage);
        return true;
    }
    bool AnthemSimpleToyRenderer::createTexture2(AnthemImage** pImage, AnthemImageCreateProps* prop, AnthemDescriptorPool* descPool, uint32_t bindLoc, uint32_t descId, bool ignoreDescPool) {
        //Allocate Image
        auto textureImage = new AnthemImage();
        textureImage->specifyLogicalDevice(this->logicalDevice.get());
        textureImage->specifyPhyDevice(this->phyDevice.get());
        textureImage->specifyCommandBuffers(this->commandBuffers.get());
        textureImage->loadImageData(prop->texData, prop->texWidth, prop->texHeight, prop->texChannel);
        textureImage->specifyUsage(prop->usage);
        if (prop->mipmap2d) {
            textureImage->enableMipMapping();
        }
        if (prop->msaa) {
            textureImage->enableMsaa();
        }
        textureImage->setImageFormat(prop->format);
        textureImage->addAccessStage(prop->extraAccessStages);
        textureImage->prepareImage();
        //Allocate Descriptor Set For Sampler
        if (descId == -1) {
            ANTH_LOGW("Descriptor pool index not specified for Tex2D, using the default value", this->imageDescPoolIdx);
            descId = this->imageDescPoolIdx;
        }
        if (!ignoreDescPool) descPool->addSampler(textureImage, bindLoc, descId);
        *pImage = textureImage;
        this->textures.push_back(textureImage);
        return true;
    }
    bool AnthemSimpleToyRenderer::createCubicTextureSimple(AnthemImageCubic** pImage, AnthemDescriptorPool* descPool, std::array<uint8_t*, 6>data,
        uint32_t texWidth, uint32_t texHeight, uint32_t texChannel, uint32_t bindLoc, uint32_t descId) {
        //Allocate Image
        auto textureImage = new AnthemImageCubic();
        textureImage->specifyLogicalDevice(this->logicalDevice.get());
        textureImage->specifyPhyDevice(this->phyDevice.get());
        textureImage->specifyCommandBuffers(this->commandBuffers.get());
        textureImage->loadImageData(data, texWidth, texHeight, texChannel);
        textureImage->specifyUsage(AT_IU_TEXTURE);
        textureImage->setImageFormat(AT_IF_SRGB_UINT8);
        textureImage->prepareImage();
        if (descId == -1) {
            ANTH_LOGW("Descriptor pool index not specified for TexCube, using the default value", this->imageDescPoolIdx);
            descId = this->imageDescPoolIdx;
        }
        descPool->addSampler(textureImage, bindLoc, descId);
        *pImage = textureImage;
        this->texCubes.push_back(textureImage);
        return true;
    }
    bool AnthemSimpleToyRenderer::createTexture3d(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth,
        uint32_t texHeight, uint32_t texDepth, uint32_t texChannel, uint32_t bindLoc, AnthemImageFormat imageFmt, uint32_t descId) {

        //Allocate Image
        auto textureImage = new AnthemImage();
        textureImage->specifyLogicalDevice(this->logicalDevice.get());
        textureImage->specifyPhyDevice(this->phyDevice.get());
        textureImage->specifyCommandBuffers(this->commandBuffers.get());
        textureImage->loadImageData3(texData, texWidth, texHeight, texChannel, texDepth);
        textureImage->specifyUsage(AnthemImageUsage::AT_IU_TEXTURE);
        textureImage->setImageFormat(imageFmt);
        textureImage->prepareImage();

        //Allocate Descriptor Set For Sampler
        if (descId == -1) {
            ANTH_LOGW("Descriptor pool index not specified for Tex3D, using the default value", this->imageDescPoolIdx);
            descId = this->imageDescPoolIdx;
        }
        descPool->addSampler(textureImage, bindLoc, descId);
        *pImage = textureImage;
        this->textures.push_back(textureImage);
        return true;
    }

    bool AnthemSimpleToyRenderer::createColorAttachmentImage(AnthemImage** pImage,AnthemDescriptorPool* descPool, uint32_t bindLoc,
        AnthemImageFormat format, bool enableMsaa,uint32_t descId){
        auto textureImage = new AnthemImage();
        textureImage->specifyLogicalDevice(this->logicalDevice.get());
        textureImage->specifyPhyDevice(this->phyDevice.get());
        textureImage->specifyCommandBuffers(this->commandBuffers.get());
        textureImage->setImageSize(this->swapChain->getSwapChainExtentWidth(),this->swapChain->getSwapChainExtentHeight());
        textureImage->specifyUsage(AT_IU_COLOR_ATTACHMENT);
        if(enableMsaa){
            textureImage->enableMsaa();
        }
        ANTH_LOGI("Creating Color Attachment");
        textureImage->setImageFormat(format);
        textureImage->prepareImage();

        //Allocate Descriptor Set For Sampler
        if(!enableMsaa){
            if (descId == -1) {
                ANTH_LOGW("Descriptor pool index not specified for ColorAttachment, using the default value", this->imageDescPoolIdx);
                descId = this->imageDescPoolIdx;
            }
            descPool->addSampler(textureImage,bindLoc, descId);
        }
        *pImage = textureImage;
        this->textures.push_back(textureImage);
        return true;
    }

    bool AnthemSimpleToyRenderer::registerPipelineSubComponents(){
        //Prepare Attr & Ind Buffers
        for(const auto& p:vertexBuffers){
            p->createBuffer();
        }
        for(const auto& p:indexBuffers){
            p->createBuffer();
        }
        for (const auto& p : indirectDrawBuffers) {
            p->createBuffer();
        }
        //for (const auto& p : ssboBuffers) {
        //    p->createShaderStorageBuffer();
        //}
        
        //Prepare Descriptor Sets
        for(const auto& p:descriptorPools){
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

    bool AnthemSimpleToyRenderer::createIndirectDrawBuffer(AnthemIndirectDrawBuffer** pIndirectBuffer) {
        auto idBuffer = new AnthemIndirectDrawBuffer();
        idBuffer->specifyLogicalDevice(this->logicalDevice.get());
        idBuffer->specifyPhyDevice(this->phyDevice.get());
        idBuffer->specifyCommandBuffers(this->commandBuffers.get());

        *pIndirectBuffer = idBuffer;
        this->indirectDrawBuffers.push_back(idBuffer);
        return true;
    }
    
    bool AnthemSimpleToyRenderer::createShader(AnthemShaderModule** pShaderModule,AnthemShaderFilePaths* filename){
        auto shaderModule = new AnthemShaderModule();
        shaderModule->createShaderModules(this->logicalDevice.get(),filename);
        *pShaderModule = shaderModule;
        this->shaders.push_back(shaderModule);
        return true;
    }
    bool AnthemSimpleToyRenderer::createGraphicsPipelineCustomized(AnthemGraphicsPipeline** pPipeline,
        std::vector<AnthemDescriptorSetEntry> descSetEntries,
        std::vector<AnthemPushConstant*> pushConstants,
        AnthemRenderPass* renderPass,
        AnthemShaderModule* shaderModule, IAnthemVertexBufferAttrLayout* vertexBuffer,
        AnthemGraphicsPipelineCreateProps* createProps){

        auto graphicsPipeline = new AnthemGraphicsPipeline();
        graphicsPipeline->specifyLogicalDevice(this->logicalDevice.get());
        graphicsPipeline->specifyViewport(this->viewport.get());
        graphicsPipeline->specifyRenderPass(renderPass);
        graphicsPipeline->specifyShaderModule(shaderModule);
        graphicsPipeline->specifyVertexBuffer(vertexBuffer);
        graphicsPipeline->specifyUniformBuffer(nullptr); 
        ANTH_TODO("Remove nullptr");
        //graphicsPipeline->specifyDescriptor(descSetEntries.at(0).descPool);
        if (createProps != nullptr) {
            graphicsPipeline->specifyProps(createProps);
        }

        graphicsPipeline->preparePreqPipelineCreateInfo();
        graphicsPipeline->createPipelineLayoutCustomized(descSetEntries, pushConstants);
        graphicsPipeline->createPipeline();
        this->graphicsPipelines.push_back(graphicsPipeline);
        *pPipeline = graphicsPipeline;
        return true;
    }
    bool AnthemSimpleToyRenderer::createComputePipelineCustomized(AnthemComputePipeline** pPipeline,std::vector<AnthemDescriptorSetEntry> descSetEntries,AnthemShaderModule* shaderModule){
        auto compPipeline = new AnthemComputePipeline();
        compPipeline->specifyLogicalDevice(this->logicalDevice.get());
        compPipeline->specifyShaderModule(shaderModule);
        compPipeline->createPipelineLayoutCustomized(descSetEntries);
        compPipeline->createPipeline();

        this->computePipelines.push_back(compPipeline);
        *pPipeline = compPipeline;
        return true;
    }
    bool AnthemSimpleToyRenderer::createSemaphore(AnthemSemaphore** pSemaphore){
        auto sp = new AnthemSemaphore();
        sp->specifyLogicalDevice(this->logicalDevice.get());
        sp->createSemaphore();
        extSemaphores.push_back(sp);
        *pSemaphore = sp;
        return true;
    }

    bool AnthemSimpleToyRenderer::createFence(AnthemFence** pFence){
        auto sp = new AnthemFence();
        sp->specifyLogicalDevice(this->logicalDevice.get());
        sp->createFence();
        extFences.push_back(sp);
        *pFence = sp;
        return true;
    }
    

    bool AnthemSimpleToyRenderer::createGraphicsPipeline(AnthemGraphicsPipeline** pPipeline,  AnthemDescriptorPool* descPool, AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule, IAnthemVertexBufferAttrLayout* vertexBuffer,AnthemUniformBuffer* uniformBuffer){
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
    bool AnthemSimpleToyRenderer::createSimpleFramebuffer(AnthemFramebuffer** pFramebuffer,const std::vector<const AnthemImage*>* colorAttachment, const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer){
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

    bool AnthemSimpleToyRenderer::addSamplerArrayToDescriptor(std::vector<AnthemImageContainer*>& images, AnthemDescriptorPool* descPool, uint32_t bindLoc, uint32_t descId) {
        if (descId == -1) {
            ANTH_LOGW("Descriptor pool index not specified for Sampler, using the default value", this->imageDescPoolIdx);
            descId = this->imageDescPoolIdx;
        }
        return descPool->addSamplerArray(images, bindLoc, descId);
    }
    bool AnthemSimpleToyRenderer::addStorageImageArrayToDescriptor(std::vector<AnthemImageContainer*>& images, AnthemDescriptorPool* descPool, uint32_t bindLoc, uint32_t descId) {
        if (descId == -1) {
            ANTH_LOGW("Descriptor pool index not specified for Sampler, using the default value", this->storageImgDescPoolIdx);
            descId = this->storageImgDescPoolIdx;
        }
        return descPool->addStorageImageArray(images, bindLoc, descId);
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
    bool AnthemSimpleToyRenderer::drAllocateCommandBuffer(uint32_t* commandBufferId){
        this->commandBuffers->createCommandBuffer(commandBufferId);
        return true;
    }
    bool AnthemSimpleToyRenderer:: drGetCommandBufferForFrame(uint32_t* commandBufferId,uint32_t frameIdx){
        *commandBufferId = this->drawingCommandHelper->getFrameCmdBufIdx(frameIdx);
        return true;
    }
    bool AnthemSimpleToyRenderer::drClearCommands(uint32_t cmdIdx){
        this->commandBuffers->resetCommandBuffer(cmdIdx);
        return true;
    }
    bool AnthemSimpleToyRenderer::drPrepareFrame(uint32_t currentFrame, uint32_t* avaImageIdx){
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
    bool AnthemSimpleToyRenderer::drSubmitCommandBufferGraphicsQueueGeneral(uint32_t cmdIdx, uint32_t frameIdx,
        const std::vector<const AnthemSemaphore*>* semaphoreToWait, const std::vector<AtSyncSemaphoreWaitStage>* semaphoreWaitStages,
        AnthemFence* customFence, bool customImageAvailableSemaphore) {
        VkFence* vf = nullptr;
        if (customFence) {
            vf = (VkFence * )customFence->getFence();
        }
        return this->mainLoopSyncer->submitCommandBufferGeneral(this->commandBuffers->getCommandBuffer(cmdIdx), frameIdx,
            semaphoreToWait, semaphoreWaitStages,vf,customImageAvailableSemaphore);
    }
    bool AnthemSimpleToyRenderer::drSubmitCommandBufferGraphicsQueueGeneral2(uint32_t cmdIdx, uint32_t frameIdx,
        const std::vector<const AnthemSemaphore*>* semaphoreToWait, const std::vector<AtSyncSemaphoreWaitStage>* semaphoreWaitStages,
        AnthemFence* customFence, const std::vector<const AnthemSemaphore*>* semaphoreToSignal) {
        VkFence vf = customFence ? (*customFence->getFence()) : nullptr;
        return this->mainLoopSyncer->submitCommandBufferGeneral2(this->commandBuffers->getCommandBuffer(cmdIdx), frameIdx,
            semaphoreToWait, semaphoreWaitStages, &vf, semaphoreToSignal);
    }
    bool AnthemSimpleToyRenderer::drPushConstants(AnthemPushConstant* pushConstant, AnthemGraphicsPipeline* pipeline, uint32_t cmdIdx) {
        vkCmdPushConstants(
            *this->commandBuffers->getCommandBuffer(cmdIdx),
            *(pipeline->getPipelineLayout()),
            pushConstant->getRange().stageFlags,
            pushConstant->getRange().offset,
            pushConstant->getSize(),
            pushConstant->getData()
            );
        return true;
    }

    bool AnthemSimpleToyRenderer::drSubmitCommandBufferCompQueueGeneral(uint32_t cmdIdx, const std::vector<const AnthemSemaphore*>* semaphoreToWait, const std::vector<const AnthemSemaphore*>* semaphoreToSignal, const AnthemFence* fenceToSignal) {
        
        VkSubmitInfo submitInfo{};
        std::vector<VkSemaphore> semToSignal;
        if (semaphoreToSignal != nullptr) {
            for (const auto& p : *semaphoreToSignal) {
                semToSignal.push_back(*p->getSemaphore());
            }
        }

        std::vector<VkSemaphore> semToWait;
        std::vector<VkPipelineStageFlags> waitStages;
        if (semaphoreToWait != nullptr) {
            for (const auto& p : *semaphoreToWait) {
                semToWait.push_back(*p->getSemaphore());
                waitStages.push_back(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            }
        }

        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = this->commandBuffers->getCommandBuffer(cmdIdx);
        submitInfo.signalSemaphoreCount = static_cast<decltype(submitInfo.signalSemaphoreCount)>(semToSignal.size());
        submitInfo.pSignalSemaphores = semToSignal.data();
        
        submitInfo.pWaitSemaphores = semToWait.data();
        submitInfo.waitSemaphoreCount = static_cast<decltype(submitInfo.signalSemaphoreCount)>(semToWait.size());
        submitInfo.pWaitDstStageMask = waitStages.data();

        if (fenceToSignal == nullptr) {
            if (vkQueueSubmit(this->logicalDevice->getComputeQueue(), 1, &submitInfo, nullptr) != VK_SUCCESS) {
                ANTH_LOGE("failed to submit compute command buffer!");
            };
        }
        else {
            if (vkQueueSubmit(this->logicalDevice->getComputeQueue(), 1, &submitInfo, *fenceToSignal->getFence()) != VK_SUCCESS) {
                ANTH_LOGE("failed to submit compute command buffer!");
            };
        }

        return true;
    }
    bool AnthemSimpleToyRenderer::drEndCommandRecording(uint32_t cmdIdx){
        this->commandBuffers->endCommandRecording(cmdIdx);
        //this->drawingCommandHelper->endCommandBuffer(frameIdx);
        return true;
    }
    bool AnthemSimpleToyRenderer::drStartCommandRecording(uint32_t cmdIdx){
        this->commandBuffers->startCommandRecording(cmdIdx);
        //this->drawingCommandHelper->startCommandBuffer(frameIdx);
        return true;
    }
    bool AnthemSimpleToyRenderer::drStartRenderPass(AnthemRenderPass* renderPass,AnthemFramebuffer* framebuffer ,uint32_t cmdIdx,bool enableMsaa){
        AnthemCommandManagerRenderPassStartInfo startInfo = {
            .renderPass = renderPass,
            .framebufferList = framebuffer,
            .framebufferIdx = 0,
            .clearValue = {{{0.0f,0.0f,0.0f,1.0f}}},
            .depthClearValue = {{1.0f, 0}},
            .usingMsaa = enableMsaa
        };
        //this->drawingCommandHelper->startRenderPass(&startInfo,frameIdx);
        ANTH_ASSERT(swapChain != nullptr,"Swap chain not specified");
        
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = *(startInfo.renderPass->getRenderPass());
        renderPassBeginInfo.framebuffer = *(startInfo.framebufferList->getFramebuffer());
        renderPassBeginInfo.renderArea.offset = {0,0};
        renderPassBeginInfo.renderArea.extent = *(swapChain->getSwapChainExtent());
        
        auto renderPassClearValue = startInfo.renderPass->getDefaultClearValue();
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(renderPassClearValue->size());
        renderPassBeginInfo.pClearValues = renderPassClearValue->data();

        ANTH_LOGV("Starting render pass");
        auto cmdBuf = commandBuffers->getCommandBuffer(cmdIdx);
        vkCmdBeginRenderPass(*cmdBuf,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
        ANTH_LOGV("Render pass started");
        return true;
    }
    bool AnthemSimpleToyRenderer::drEndRenderPass(uint32_t cmdIdx){
        //this->drawingCommandHelper->endRenderPass(frameIdx);
        auto cmdBuf = commandBuffers->getCommandBuffer(cmdIdx);
        vkCmdEndRenderPass(*cmdBuf);
        return true;
    }
    bool AnthemSimpleToyRenderer::drSubmitBufferPrimaryCall(uint32_t frameIdx,uint32_t cmdIdx){
        this->mainLoopSyncer->submitCommandBuffer(this->commandBuffers->getCommandBuffer(cmdIdx),frameIdx);
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

    bool AnthemSimpleToyRenderer::drSetViewportScissor(uint32_t cmdIdx){
        vkCmdSetViewport(*this->commandBuffers->getCommandBuffer(cmdIdx),0,1,viewport->getViewport());
        vkCmdSetScissor(*this->commandBuffers->getCommandBuffer(cmdIdx),0,1,viewport->getScissor());
        return true;
    }
    bool AnthemSimpleToyRenderer::drSetLineWidth(float lineWidth, uint32_t cmdIdx) {
        vkCmdSetLineWidth(*this->commandBuffers->getCommandBuffer(cmdIdx), lineWidth);
        return true;
    }

    bool AnthemSimpleToyRenderer::drBindGraphicsPipeline(AnthemGraphicsPipeline* pipeline,uint32_t cmdIdx){
        vkCmdBindPipeline(*this->commandBuffers->getCommandBuffer(cmdIdx),VK_PIPELINE_BIND_POINT_GRAPHICS,*(pipeline->getPipeline()));
        return true;
    }
    bool AnthemSimpleToyRenderer::drBindComputePipeline(AnthemComputePipeline* pipeline,uint32_t cmdIdx){
        vkCmdBindPipeline(*this->commandBuffers->getCommandBuffer(cmdIdx),VK_PIPELINE_BIND_POINT_COMPUTE,*(pipeline->getPipeline()));
        return true;
    }

    bool AnthemSimpleToyRenderer::drBindVertexBuffer(AnthemVertexBuffer* vertexBuffer,uint32_t cmdIdx){
        vkCmdBindVertexBuffers(*this->commandBuffers->getCommandBuffer(cmdIdx),0,1,(vertexBuffer->getDestBufferObject()),emptyOffsetPlaceholder);
        return true;
    }

    bool AnthemSimpleToyRenderer::drBindVertexBufferMultiple(std::vector<AnthemVertexBuffer*> vertexBuffer, uint32_t cmdIdx) {
        std::vector<VkBuffer> bufs;
        std::vector<VkDeviceSize> vdSize;
        for (auto p : vertexBuffer) {
            bufs.push_back(*p->getDestBufferObject());
            vdSize.push_back(0);
        }
        vkCmdBindVertexBuffers(*this->commandBuffers->getCommandBuffer(cmdIdx), 0, vertexBuffer.size(), bufs.data(), vdSize.data());
        return true;
    }

    bool AnthemSimpleToyRenderer::drBindVertexBufferFromSsbo(AnthemShaderStorageBuffer* vertexBuffer, uint32_t copyId, uint32_t cmdIdx) {
        vkCmdBindVertexBuffers(*this->commandBuffers->getCommandBuffer(cmdIdx), 0, 1, (vertexBuffer->getDestBufferObject(copyId)), emptyOffsetPlaceholder);
        return true;
    }
    bool AnthemSimpleToyRenderer::drBindIndexBuffer(AnthemIndexBuffer* indexBuffer,uint32_t cmdIdx){
        vkCmdBindIndexBuffer(*this->commandBuffers->getCommandBuffer(cmdIdx), *(indexBuffer->getDestBufferObject()), 0, VK_INDEX_TYPE_UINT32);
        return true;
    }
    bool AnthemSimpleToyRenderer::drBindDescriptorSetCustomizedCompute(std::vector<AnthemDescriptorSetEntry> descSetEntries, AnthemComputePipeline* pipeline, uint32_t cmdIdx) {
        std::vector<VkDescriptorSet>* descSets = new std::vector<VkDescriptorSet>();
        for (const auto& p : descSetEntries) {
            if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER) {
                p.descPool->appendDescriptorSetSampler(p.inTypeIndex, descSets);
            }
            else if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER) {
                p.descPool->appendDescriptorSetUniform(p.inTypeIndex, descSets);
            }
            else if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_SHADER_STORAGE_BUFFER) {
                p.descPool->appendDescriptorSetSsbo(p.inTypeIndex, descSets);
            }
            else if (p.descSetType == AnthemDescriptorSetEntrySourceType::AT_ACDS_STORAGE_IMAGE) {
                p.descPool->appendDescriptorSetStorageImage(p.inTypeIndex, descSets);
            }
            else {
                ANTH_LOGE("Unknown Descriptor Set Type");
            }
        }
        vkCmdBindDescriptorSets(*this->commandBuffers->getCommandBuffer(cmdIdx), VK_PIPELINE_BIND_POINT_COMPUTE, *(pipeline->getPipelineLayout()), 0,
            static_cast<uint32_t>(descSets->size()), descSets->data(), 0, nullptr);
        delete descSets;
        return true;
    }
    bool AnthemSimpleToyRenderer::drBindDescriptorSetCustomizedGraphics(std::vector<AnthemDescriptorSetEntry> descSetEntries, AnthemGraphicsPipeline* pipeline, uint32_t cmdIdx){
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
        vkCmdBindDescriptorSets(*this->commandBuffers->getCommandBuffer(cmdIdx), VK_PIPELINE_BIND_POINT_GRAPHICS, *(pipeline->getPipelineLayout()), 0,
            static_cast<uint32_t>(descSets->size()),descSets->data() , 0, nullptr);
        delete descSets;
        return true;
    }

    bool AnthemSimpleToyRenderer::drColorImagePipelineBarrier(AnthemImageContainer* container, AnthemImagePipelineBarrier* srcProp,
        AnthemImagePipelineBarrier* dstProp, uint32_t cmdIdx) {
        auto p = this->commandBuffers->getCommandBuffer(cmdIdx);
        auto x = (std::remove_cv_t<std::remove_pointer_t<decltype(p)>>*)(p);
        return container->recordPipelineBarrier(x, srcProp, dstProp, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    bool AnthemSimpleToyRenderer::drStorageBufferPipelineBarrier(AnthemShaderStorageBuffer* buffer, uint32_t copyId, AnthemBufferBarrierProp* src, AnthemBufferBarrierProp* dst, uint32_t cmdIdx){
        return buffer->setupBarrier(*this->commandBuffers->getCommandBuffer(cmdIdx), copyId, src, dst);
    }

    bool AnthemSimpleToyRenderer::drBindDescriptorSet(AnthemDescriptorPool* descPool, AnthemGraphicsPipeline* pipeline, uint32_t frameIdx, uint32_t cmdIdx){
        std::vector<VkDescriptorSet>* descSets = new std::vector<VkDescriptorSet>();
        descPool->getAllDescriptorSets(frameIdx,descSets);
        vkCmdBindDescriptorSets(*this->commandBuffers->getCommandBuffer(cmdIdx), VK_PIPELINE_BIND_POINT_GRAPHICS, *(pipeline->getPipelineLayout()), 0,
            static_cast<uint32_t>(descSets->size()),descSets->data() , 0, nullptr);
        delete descSets;
        return true;
    }
    bool AnthemSimpleToyRenderer::drDraw(uint32_t vertices,uint32_t cmdIdx){
        vkCmdDrawIndexed(*this->commandBuffers->getCommandBuffer(cmdIdx), vertices, 1, 0, 0, 0);
        return true;
    }
    bool AnthemSimpleToyRenderer::drDrawInstanced(uint32_t vertices, uint32_t instances, uint32_t cmdIdx) {
        vkCmdDrawIndexed(*this->commandBuffers->getCommandBuffer(cmdIdx), vertices, instances, 0, 0, 0);
        return true;
    }
    bool AnthemSimpleToyRenderer::drDrawInstancedAll(uint32_t vertices, uint32_t instances, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance, uint32_t cmdIdx) {
        vkCmdDrawIndexed(*this->commandBuffers->getCommandBuffer(cmdIdx), vertices, instances,firstIndex,vertexOffset,firstInstance);
        return true;
    }
    bool AnthemSimpleToyRenderer::drDrawIndexedIndirect(AnthemIndirectDrawBuffer* buffer, uint32_t cmdIdx) {
        auto feats = (*this->phyDevice).getDeviceFeatures();
        if (feats.multiDrawIndirect == VK_TRUE) {
            vkCmdDrawIndexedIndirect(*this->commandBuffers->getCommandBuffer(cmdIdx),
                *buffer->getBuffer(), 0, buffer->getNumCommands(), sizeof(VkDrawIndexedIndirectCommand));
        }
        else {
            ANTH_LOGW("Multi indirect draw not supported");
            for (int i = 0; i < buffer->getNumCommands(); i++) {
                vkCmdDrawIndexedIndirect(*this->commandBuffers->getCommandBuffer(cmdIdx),
                    *buffer->getBuffer(), i * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
            }
        }
        return true;
    }
    bool AnthemSimpleToyRenderer::drDrawMesh(uint32_t groupX, uint32_t groupY, uint32_t groupZ, uint32_t cmdIdx) {
        this->logicalDevice->vkCall_vkCmdDrawMeshTaskExt(*this->commandBuffers->getCommandBuffer(cmdIdx), groupX, groupY, groupZ);
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
    bool AnthemSimpleToyRenderer::createDepthBufferWithSampler(AnthemDepthBuffer** pDepthBuffer,AnthemDescriptorPool* descPool, uint32_t bindLoc, bool enableMsaa){
        auto depthBuffer = new AnthemDepthBuffer();
        depthBuffer->specifyLogicalDevice(this->logicalDevice.get());
        depthBuffer->specifyPhyDevice(this->phyDevice.get());
        depthBuffer->specifyCommandBuffers(this->commandBuffers.get());
        depthBuffer->specifySwapChain(this->swapChain.get());
        if(enableMsaa){
            depthBuffer->enableMsaa();
        }
        depthBuffer->createDepthBufferWithSampler();
        descPool->addSampler(depthBuffer,bindLoc,this->imageDescPoolIdx);
        this->depthBuffers.push_back(depthBuffer);
        *pDepthBuffer = depthBuffer;
        return true;
    }
    bool AnthemSimpleToyRenderer::createDepthBuffer(AnthemDepthBuffer** pDepthBuffer, bool enableMsaa){
        auto depthBuffer = new AnthemDepthBuffer();
        depthBuffer->specifyLogicalDevice(this->logicalDevice.get());
        depthBuffer->specifyPhyDevice(this->phyDevice.get());
        depthBuffer->specifyCommandBuffers(this->commandBuffers.get());
        depthBuffer->specifySwapChain(this->swapChain.get());
        if(enableMsaa){
            depthBuffer->enableMsaa();
        }
        depthBuffer->createDepthBuffer();
        this->depthBuffers.push_back(depthBuffer);
        
        *pDepthBuffer = depthBuffer;
        return true;
    }
    bool AnthemSimpleToyRenderer::drComputeDispatch(uint32_t cmdIdx, uint32_t workgroupX, uint32_t workgroupY, uint32_t workgroupZ) {
        vkCmdDispatch(*this->commandBuffers->getCommandBuffer(cmdIdx), workgroupX, workgroupY, workgroupZ);
        return true;
    }
    bool AnthemSimpleToyRenderer::exGetWindowSize(int& height,int& width){
        height = this->windowHeight;
        width = this->windowWidth;
        return true;
    }
    AnthemSimpleToyRenderer::~AnthemSimpleToyRenderer(){
        if(this->setupState){
            this->finalize();
        }
    }
    bool AnthemSimpleToyRenderer::quGetComputeQueueIdx(uint32_t* queue) {
        *queue = phyDevice->getPhyQueueComputeFamilyIndice().value();
        return true;
    }
    bool AnthemSimpleToyRenderer::quGetGraphicsQueueIdx(uint32_t* queue) {
        *queue = phyDevice->getPhyQueueGraphicsFamilyIndice().value();
        return true;
    }

}