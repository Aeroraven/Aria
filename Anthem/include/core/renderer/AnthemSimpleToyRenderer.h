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
#include "../drawing/AnthemImage.h"
#include "../drawing/AnthemVertexBuffer.h"
#include "../drawing/AnthemUniformBuffer.h"
#include "../drawing/AnthemDescriptorPool.h"
#include "../drawing/AnthemIndexBuffer.h"
#include "../drawing/AnthemMainLoopSyncer.h"
#include "../drawing/AnthemFramebufferList.h"
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
        ANTH_UNIQUE_PTR(AnthemDescriptorPool) descriptorPool;
        ANTH_UNIQUE_PTR(AnthemMainLoopSyncer) mainLoopSyncer;
        ANTH_UNIQUE_PTR(AnthemViewport) viewport;

        ANTH_UNIQUE_PTR(AnthemFramebufferList) framebufferList;

        ANTH_UNIQUE_PTR(AnthemDrawingCommandHelper) drawingCommandHelper;


        std::vector<AnthemRenderPass*> renderPasses;
        std::vector<AnthemImage*> textures;
        std::vector<AnthemShaderModule*> shaders;

        std::vector<AnthemVertexBuffer*> vertexBuffers;
        std::vector<AnthemIndexBuffer*> indexBuffers;
        std::vector<AnthemUniformBuffer*> uniformBuffers;

        std::vector<AnthemFramebufferList*> framebufferListObjs;
        std::vector<AnthemDepthBuffer*> depthBuffers;

        std::vector<AnthemGraphicsPipeline*> graphicsPipelines;

        uint32_t uniformDescPoolIdx = -1;
        uint32_t imageDescPoolIdx = -1;

    private:
        bool resizeRefreshState = false;
        std::function<void()> drawLoopHandler = nullptr;

    protected:
        bool destroySwapChain();
        bool recreateSwapChain();

    public:
        bool setConfig(const AnthemConfig* config);
        bool initialize();
        bool finialize();

        bool startDrawLoopDemo();
        bool setDrawFunction(std::function<void()> drawLoopHandler);

        bool prepareFrame(uint32_t currentFrame, uint32_t* avaImageIdx);
        bool presentFrameDemo(uint32_t currentFrame, AnthemRenderPass* renderPass, 
            AnthemGraphicsPipeline* pipeline, AnthemFramebufferList* framebuffer,uint32_t avaImageIdx,
            AnthemVertexBuffer* vbuf, AnthemUniformBuffer* ubuf,AnthemIndexBuffer* ibuf);

        bool setupDemoRenderPass(AnthemRenderPass** pRenderPass, AnthemDepthBuffer* depthBuffer);
        bool createDepthBuffer(AnthemDepthBuffer** pDepthBuffer);
        bool createTexture(AnthemImage** pImage, uint8_t* texData, uint32_t texWidth,uint32_t texHeight, uint32_t texChannel, uint32_t bindLoc);
        bool createIndexBuffer(AnthemIndexBuffer** pIndexBuffer);
        bool createShader(AnthemShaderModule** pShaderModule,AnthemShaderFilePaths* filename);
        bool createFramebufferList(AnthemFramebufferList** pFramebufferList,const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer);

        bool registerPipelineSubComponents();
        bool createPipeline(AnthemGraphicsPipeline** pPipeline,AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule,AnthemVertexBuffer* vertexBuffer,AnthemUniformBuffer* uniformBuffer);

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

        template<typename... T,uint32_t... Rk,uint32_t... Sz>
        bool createUniformBuffer(AnthemUniformBufferImpl<AnthemUBDesc<T,Rk,Sz>...>** pUniformBuffer, uint32_t bindLoc){
            //Allocate Uniform Buffer
            auto ubuf = new AnthemUniformBufferImpl<AnthemUBDesc<T,Rk,Sz>...>();
            ubuf->specifyLogicalDevice(this->logicalDevice.get());
            ubuf->specifyPhyDevice(this->phyDevice.get());
            ubuf->createBuffer(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT);

            //Allocate Descriptor Set
            this->descriptorPool->addUniformBuffer(ubuf,bindLoc,this->uniformDescPoolIdx);

            *pUniformBuffer = ubuf;
            this->uniformBuffers.push_back(ubuf);
            return true;
        }

        

    };
}