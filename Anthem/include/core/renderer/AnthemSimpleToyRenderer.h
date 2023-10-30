#pragma once
#include "../base/AnthemBaseImports.h"
#include "../base/AnthemInstance.h"
#include "../base/AnthemValLayer.h"
#include "../base/AnthemConfig.h"
#include "../base/AnthemWindowSurface.h"
#include "../base/AnthemPhyDeviceSelector.h"
#include "../base/AnthemLogicalDeviceSelector.h"
#include "../base/AnthemSwapChain.h"
#include "../drawing/AnthemCommandBuffers.h"
#include "../drawing/AnthemDescriptorPool.h"
#include "../drawing/AnthemMainLoopSyncer.h"
#include "../drawing/AnthemSwapchainFramebuffer.h"
#include "../drawing/AnthemFramebuffer.h"
#include "../pipeline/AnthemViewport.h"
#include "../pipeline/AnthemShaderModule.h"

#include "../pipeline/AnthemRenderPass.h"
#include "../pipeline/AnthemGraphicsPipeline.h"
#include "../drawing/AnthemDrawingCommandHelper.h"

namespace Anthem::Core{
    

    class AnthemSimpleToyRenderer{
    private:
        const AnthemConfig* config = nullptr;
        ANTH_UNIQUE_PTR(AnthemInstance) instance;
        ANTH_UNIQUE_PTR(AnthemValLayer) validationLayer;
        ANTH_UNIQUE_PTR(AnthemWindowSurface) windowSurface;
        ANTH_UNIQUE_PTR(AnthemPhyDeviceSelector) phyDeviceSelector;
        ANTH_UNIQUE_PTR(AnthemSwapChain) swapChain;
        ANTH_UNIQUE_PTR(AnthemPhyDevice) phyDevice;
        ANTH_UNIQUE_PTR(AnthemLogicalDeviceSelector) logicalDeviceSelector;
        ANTH_UNIQUE_PTR(AnthemLogicalDevice) logicalDevice;
        ANTH_UNIQUE_PTR(AnthemCommandBuffers) commandBuffers;
        ANTH_UNIQUE_PTR(AnthemMainLoopSyncer) mainLoopSyncer;
        ANTH_UNIQUE_PTR(AnthemViewport) viewport;

        ANTH_UNIQUE_PTR(AnthemSwapchainFramebuffer) framebufferList;

        ANTH_UNIQUE_PTR(AnthemDrawingCommandHelper) drawingCommandHelper;


        std::vector<AnthemRenderPass*> renderPasses;
        std::vector<AnthemImage*> textures;
        std::vector<AnthemShaderModule*> shaders;

        std::vector<AnthemVertexBuffer*> vertexBuffers;
        std::vector<AnthemIndexBuffer*> indexBuffers;
        std::vector<AnthemUniformBuffer*> uniformBuffers;

        std::vector<AnthemSwapchainFramebuffer*> framebufferListObjs;
        std::vector<AnthemFramebuffer*> simpleFramebuffers;
        std::vector<AnthemDepthBuffer*> depthBuffers;
        std::vector<AnthemDescriptorPool*> descriptorPools;

        std::vector<AnthemGraphicsPipeline*> graphicsPipelines;

        uint32_t uniformDescPoolIdx = -1;
        uint32_t imageDescPoolIdx = -1;

        VkDeviceSize emptyOffsetPlaceholder[1] = {0};

    private:
        int windowWidth = 0;
        int windowHeight = 0;
        bool resizeRefreshState = false;
        bool setupState = false;
        std::function<void()> drawLoopHandler = nullptr;

    protected:
        bool destroySwapChain();
        bool recreateSwapChain();

    public:
        ~AnthemSimpleToyRenderer();

        bool setConfig(const AnthemConfig* config);
        bool initialize();
        bool finalize();

        bool startDrawLoopDemo();
        bool setDrawFunction(std::function<void()> drawLoopHandler);

        bool drPrepareFrame(uint32_t currentFrame, uint32_t* avaImageIdx);
        bool presentFrameDemo(uint32_t currentFrame, AnthemRenderPass* renderPass, 
            AnthemGraphicsPipeline* pipeline, AnthemSwapchainFramebuffer* framebuffer,uint32_t avaImageIdx,
            AnthemVertexBuffer* vbuf, AnthemUniformBuffer* ubuf,AnthemIndexBuffer* ibuf, AnthemDescriptorPool* descPool);

        bool setupDemoRenderPass(AnthemRenderPass** pRenderPass, AnthemDepthBuffer* depthBuffer);
        bool setupRenderPass(AnthemRenderPass** pRenderPass, AnthenRenderPassSetupOption* setupOption, AnthemDepthBuffer* depthBuffer);
        bool createDepthBuffer(AnthemDepthBuffer** pDepthBuffer, bool enableMsaa);
        bool createTexture(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth,uint32_t texHeight,
             uint32_t texChannel, uint32_t bindLoc, bool generateMipmap, bool enableMsaa);
        bool createColorAttachmentImage(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint32_t bindLoc,  AnthemImageFormat format, bool enableMsaa);
        bool createIndexBuffer(AnthemIndexBuffer** pIndexBuffer);
        bool createShader(AnthemShaderModule** pShaderModule,AnthemShaderFilePaths* filename);
        bool createSwapchainImageFramebuffers(AnthemSwapchainFramebuffer** pFramebufferList,const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer);
        bool createSimpleFramebuffer(AnthemFramebuffer** pFramebuffer, const std::vector<const AnthemImage*>* colorAttachment, const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer);
        bool createDescriptorPool(AnthemDescriptorPool** pDescriptorPool);

        bool registerPipelineSubComponents();
        bool createPipeline(AnthemGraphicsPipeline** pPipeline,  AnthemDescriptorPool* descPool, AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule,AnthemVertexBuffer* vertexBuffer,AnthemUniformBuffer* uniformBuffer);
        bool createPipelineCustomized(AnthemGraphicsPipeline** pPipeline,std::vector<AnthemDescriptorSetEntry> descSetEntries,AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule,AnthemVertexBuffer* vertexBuffer);

        bool drStartCommandRecording(uint32_t frameIdx);
        bool drEndCommandRecording(uint32_t frameIdx);
        bool drStartRenderPass(AnthemRenderPass* renderPass,AnthemFramebuffer* framebufferList ,uint32_t frameIdx,bool enableMsaa);
        bool drEndRenderPass(uint32_t frameIdx);
        bool drPresentFrame(uint32_t frameIdx, uint32_t avaImageIdx);
        bool drSubmitBuffer(uint32_t frameIdx);
        bool drClearCommands(uint32_t frameIdx);

        bool drSetViewportScissor(uint32_t frameIdx);
        bool drBindPipeline(AnthemGraphicsPipeline* pipeline,uint32_t frameIdx);
        bool drBindVertexBuffer(AnthemVertexBuffer* vertexBuffer,uint32_t frameIdx);
        bool drBindIndexBuffer(AnthemIndexBuffer* indexBuffer,uint32_t frameIdx);
        bool drBindDescriptorSet(AnthemDescriptorPool* descPool, AnthemGraphicsPipeline* pipeline, uint32_t frameIdx);
        bool drBindDescriptorSetCustomized(std::vector<AnthemDescriptorSetEntry> descSetEntries, AnthemGraphicsPipeline* pipeline, uint32_t frameIdx);
        bool drDraw(uint32_t vertices,uint32_t frameIdx);

        bool exGetWindowSize(int& height,int& width);

        template<typename... T,uint32_t... S>
        bool createVertexBuffer(AnthemVertexBufferImpl<AnthemVAOAttrDesc<T,S>...>** pVertexBuffer){
            auto vxBufferImpl = new AnthemVertexBufferImpl<AnthemVAOAttrDesc<T,S>...>();
            vxBufferImpl->specifyLogicalDevice(this->logicalDevice.get());
            vxBufferImpl->specifyPhyDevice(this->phyDevice.get());
            vxBufferImpl->specifyCommandBuffers(this->commandBuffers.get());
            *pVertexBuffer = vxBufferImpl;
            this->vertexBuffers.push_back(vxBufferImpl);
            return true;
        }

        template<typename... T,uint32_t... Rk,uint32_t... Sz,uint32_t... ArrSz>
        bool createUniformBuffer(AnthemUniformBufferImpl<AnthemUBDesc<T,Rk,Sz,ArrSz>...>** pUniformBuffer, uint32_t bindLoc, AnthemDescriptorPool *descPool){
            //Allocate Uniform Buffer
            auto ubuf = new AnthemUniformBufferImpl<AnthemUBDesc<T,Rk,Sz,ArrSz>...>();
            ubuf->specifyLogicalDevice(this->logicalDevice.get());
            ubuf->specifyPhyDevice(this->phyDevice.get());
            ubuf->createBuffer(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT);

            //Allocate Descriptor Set
            descPool->addUniformBuffer(ubuf,bindLoc,this->uniformDescPoolIdx);

            *pUniformBuffer = ubuf;
            this->uniformBuffers.push_back(ubuf);
            return true;
        }

        

    };
}