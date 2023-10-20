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
#include "../drawing/AnthemFramebufferList.h"
#include "../pipeline/AnthemViewport.h"
#include "../pipeline/AnthemShaderModule.h"

#include "../pipeline/AnthemRenderPass.h"
#include "../pipeline/AnthemGraphicsPipeline.h"
#include "../drawing/AnthemDrawingCommandHelper.h"

namespace Anthem::Core{
    enum AnthemCmdDescriptorSetEntrySourceType{
        AT_ACDS_UNDEFINED = 0,
        AT_ACDS_UNIFORM_BUFFER = 1,
        AT_ACDS_SAMPLER = 2,
    };
    struct AnthemCmdDescriptorSetEntry{
        AnthemDescriptorPool* descPool = nullptr;
        AnthemCmdDescriptorSetEntrySourceType descSetType = AT_ACDS_UNDEFINED;
        uint32_t inTypeIndex = 0;
    };

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
        std::vector<AnthemDescriptorPool*> descriptorPools;

        std::vector<AnthemGraphicsPipeline*> graphicsPipelines;

        uint32_t uniformDescPoolIdx = -1;
        uint32_t imageDescPoolIdx = -1;

        VkDeviceSize emptyOffsetPlaceholder[1] = {0};

    private:
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
        bool finialize();

        bool startDrawLoopDemo();
        bool setDrawFunction(std::function<void()> drawLoopHandler);

        bool prepareFrame(uint32_t currentFrame, uint32_t* avaImageIdx);
        bool presentFrameDemo(uint32_t currentFrame, AnthemRenderPass* renderPass, 
            AnthemGraphicsPipeline* pipeline, AnthemFramebufferList* framebuffer,uint32_t avaImageIdx,
            AnthemVertexBuffer* vbuf, AnthemUniformBuffer* ubuf,AnthemIndexBuffer* ibuf, AnthemDescriptorPool* descPool);

        bool setupDemoRenderPass(AnthemRenderPass** pRenderPass, AnthemDepthBuffer* depthBuffer);
        bool createDepthBuffer(AnthemDepthBuffer** pDepthBuffer);
        bool createTexture(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth,uint32_t texHeight, uint32_t texChannel, uint32_t bindLoc);
        bool createIndexBuffer(AnthemIndexBuffer** pIndexBuffer);
        bool createShader(AnthemShaderModule** pShaderModule,AnthemShaderFilePaths* filename);
        bool createFramebufferList(AnthemFramebufferList** pFramebufferList,const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer);
        bool createDescriptorPool(AnthemDescriptorPool** pDescriptorPool);

        bool registerPipelineSubComponents();
        bool createPipeline(AnthemGraphicsPipeline** pPipeline,  AnthemDescriptorPool* descPool, AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule,AnthemVertexBuffer* vertexBuffer,AnthemUniformBuffer* uniformBuffer);

        bool drStartRenderPass(AnthemRenderPass* renderPass,AnthemFramebufferList* framebufferList, uint32_t avaImgIdx ,uint32_t frameIdx);
        bool drEndRenderPass(uint32_t frameIdx);
        bool drPresentFrame(uint32_t frameIdx, uint32_t avaImageIdx);
        bool drSubmitBuffer(uint32_t frameIdx);

        bool drSetViewportScissor(uint32_t frameIdx);
        bool drBindPipeline(AnthemGraphicsPipeline* pipeline,uint32_t frameIdx);
        bool drBindVertexBuffer(AnthemVertexBuffer* vertexBuffer,uint32_t frameIdx);
        bool drBindIndexBuffer(AnthemIndexBuffer* indexBuffer,uint32_t frameIdx);
        bool drBindDescriptorSet(AnthemDescriptorPool* descPool, AnthemGraphicsPipeline* pipeline, uint32_t frameIdx);
        bool drBindDescriptorSetCustomized(std::vector<AnthemCmdDescriptorSetEntry> descSetEntries, AnthemGraphicsPipeline* pipeline, uint32_t frameIdx);
        bool drDraw(uint32_t vertices,uint32_t frameIdx);

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
        bool createUniformBuffer(AnthemUniformBufferImpl<AnthemUBDesc<T,Rk,Sz>...>** pUniformBuffer, uint32_t bindLoc, AnthemDescriptorPool *descPool){
            //Allocate Uniform Buffer
            auto ubuf = new AnthemUniformBufferImpl<AnthemUBDesc<T,Rk,Sz>...>();
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