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
#include "../pipeline/AnthemComputePipeline.h"
#include "../drawing/AnthemDrawingCommandHelper.h"

#include "../drawing/synchronization/AnthemFence.h"
#include "../drawing/synchronization/AnthemSemaphore.h"

#include "../drawing/buffer/impl/AnthemShaderStorageBufferImpl.h"
#include "../drawing/buffer/impl/AnthemUniformBufferImpl.h"
#include "../drawing/buffer/impl/AnthemVertexBufferImpl.h"
#include "../drawing/buffer/impl/AnthemInstancingVertexBufferImpl.h"
#include "../drawing/buffer/impl/AnthemPushConstantImpl.h"

#include "../drawing/buffer/AnthemIndirectDrawBuffer.h"
#include "../drawing/image/AnthemImageCubic.h"

#ifdef AT_FEATURE_RAYTRACING_ENABLED
#include "../pipeline/raytracing/AnthemRayTracingPipeline.h"
#include "../pipeline/raytracing/AnthemRayTracingShaders.h"
#include "../drawing/buffer/acceleration/AnthemAccelerationStruct.h"
#include "../drawing/buffer/acceleration/AnthemAccStructGeometry.h"
#include "../drawing/buffer/acceleration/AnthemAccStructInstance.h"
#include "../drawing/buffer/acceleration/AnthemBottomLevelAccStruct.h"
#include "../drawing/buffer/acceleration/AnthemTopLevelAccStruct.h"
#endif

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>



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

#ifdef AT_FEATURE_RAYTRACING_ENABLED
        std::vector<AnthemRayTracingShaders*> rtShaders;
        std::vector<AnthemRayTracingPipeline*> rtPipelines;
        std::vector<AnthemTopLevelAccStruct*> rtTlasList;
        std::vector<AnthemBottomLevelAccStruct*> rtBlasList;
        std::vector<AnthemAccStructGeometry*> rtGeometries;
        std::vector<AnthemAccStructInstance*> rtInstances;
#endif


        std::vector<AnthemRenderPass*> renderPasses;
        std::vector<AnthemImage*> textures;
        std::vector<AnthemImageCubic*> texCubes;
        std::vector<AnthemShaderModule*> shaders;

        std::vector<AnthemVertexBuffer*> vertexBuffers;
        std::vector<AnthemIndexBuffer*> indexBuffers;
        std::vector<AnthemIndirectDrawBuffer*> indirectDrawBuffers;
        std::vector<AnthemUniformBuffer*> uniformBuffers;
        std::vector<AnthemShaderStorageBuffer*> ssboBuffers;

        std::vector<AnthemSwapchainFramebuffer*> framebufferListObjs;
        std::vector<AnthemFramebuffer*> simpleFramebuffers;
        std::vector<AnthemDepthBuffer*> depthBuffers;
        std::vector<AnthemDescriptorPool*> descriptorPools;

        std::vector<AnthemGraphicsPipeline*> graphicsPipelines;
        std::vector<AnthemComputePipeline*> computePipelines;

        std::vector<AnthemFence*> extFences;
        std::vector<AnthemSemaphore*> extSemaphores;
        std::vector<AnthemPushConstant*> pushConstants;

        std::vector<AnthemViewport*> customViewports;


        uint32_t uniformDescPoolIdx = -1;
        uint32_t imageDescPoolIdx = -1;
        uint32_t ssboDescPoolIdx = -1;
        uint32_t storageImgDescPoolIdx = -1;

        VkDeviceSize emptyOffsetPlaceholder[1] = {0};

    private: //For IMGUI
        VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;
        VkRenderPass g_RenderPass = VK_NULL_HANDLE;
        AnthemSemaphore** g_RenderComplete = nullptr;
        uint32_t* g_CommandBufferIdx = nullptr;

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

        // Initialization
        bool setConfig(const AnthemConfig* config);
        bool initialize();
        bool finalize();

        bool startDrawLoopDemo();
        bool setDrawFunction(std::function<void()> drawLoopHandler);

        // Object Creation

        bool setupDemoRenderPass(AnthemRenderPass** pRenderPass, AnthemDepthBuffer* depthBuffer, bool retain=false);
        bool setupRenderPass(AnthemRenderPass** pRenderPass, AnthemRenderPassSetupOption* setupOption, AnthemDepthBuffer* depthBuffer);
        bool createDepthBuffer(AnthemDepthBuffer** pDepthBuffer, bool enableMsaa);
        bool createDepthBufferWithSampler(AnthemDepthBuffer** pDepthBuffer,AnthemDescriptorPool* descPool, uint32_t bindLoc, bool enableMsaa);
        bool createDepthBufferCubicWithSampler(AnthemDepthBuffer** pDepthBuffer, AnthemDescriptorPool* descPool, uint32_t bindLoc, uint32_t height, bool enableMsaa);
        bool createViewportCustom(AnthemViewport** pViewport, float width, float height, float minDepth, float maxDepth);
        bool createTexture(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth,uint32_t texHeight,
             uint32_t texChannel, uint32_t bindLoc, bool generateMipmap2D, bool enableMsaa,
            AnthemImageFormat imageFmt = AnthemImageFormat::AT_IF_SRGB_UINT8,uint32_t descId=-1,bool ignoreDescPool=false,
            AnthemImageUsage usage = AT_IU_TEXTURE);
        bool createTexture2(AnthemImage** pImage, AnthemImageCreateProps* prop, AnthemDescriptorPool* descPool, uint32_t bindLoc, uint32_t descId = -1, bool ignoreDescPool = false);
        bool createCubicTextureSimple(AnthemImageCubic** pImage, AnthemDescriptorPool* descPool, std::array<uint8_t*, 6>data, 
            uint32_t texWidth, uint32_t texHeight, uint32_t texChannel, uint32_t bindLoc, uint32_t descId);
        bool createTexture3d(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth, uint32_t texHeight, uint32_t texDepth,
            uint32_t texChannel, uint32_t bindLoc, AnthemImageFormat imageFmt = AnthemImageFormat::AT_IF_SRGB_UINT8,uint32_t descId=-1);
        bool createColorAttachmentImage(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint32_t bindLoc,
            AnthemImageFormat format, bool enableMsaa, uint32_t descId = -1);
        bool createIndexBuffer(AnthemIndexBuffer** pIndexBuffer);
        bool createShader(AnthemShaderModule** pShaderModule,AnthemShaderFilePaths* filename);
        bool createSwapchainImageFramebuffers(AnthemSwapchainFramebuffer** pFramebufferList,const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer);
        bool createSimpleFramebuffer(AnthemFramebuffer** pFramebuffer, const std::vector<const IAnthemImageViewContainer*>* colorAttachment, const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer);
        bool createDescriptorPool(AnthemDescriptorPool** pDescriptorPool);
        bool createIndirectDrawBuffer(AnthemIndirectDrawBuffer** pBuffer);

        bool createGraphicsPipeline(AnthemGraphicsPipeline** pPipeline,  AnthemDescriptorPool* descPool, AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule, IAnthemVertexBufferAttrLayout* vertexBuffer,AnthemUniformBuffer* uniformBuffer);
        bool createGraphicsPipelineCustomized(AnthemGraphicsPipeline** pPipeline,
            std::vector<AnthemDescriptorSetEntry> descSetEntries, std::vector<AnthemPushConstant*> pushConstants,
            AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule, IAnthemVertexBufferAttrLayout* vertexBuffer,AnthemGraphicsPipelineCreateProps* createProps);
        bool createComputePipelineCustomized(AnthemComputePipeline** pPipeline,std::vector<AnthemDescriptorSetEntry> descSetEntries,AnthemShaderModule* shaderModule);
        
        bool createSemaphore(AnthemSemaphore** pSemaphore);
        bool createFence(AnthemFence** pFence);

#ifdef AT_FEATURE_RAYTRACING_ENABLED
        bool createTopLevelAS(AnthemTopLevelAccStruct** pTlas);
        bool createBottomLevelAS(AnthemBottomLevelAccStruct** pBlas);
        bool createRayTracingPipeline(AnthemRayTracingPipeline** pPipeline, const std::vector<AnthemDescriptorSetEntry>& descriptors,
            const std::vector<AnthemPushConstant*> pconst, AnthemRayTracingShaders* shader,uint32_t rayRecursion);
        bool createRayTracingGeometry(AnthemAccStructGeometry** pAsGeo, uint32_t vertexStride, std::vector<float> vertices, std::vector<uint32_t>indices, std::vector<float>transform);
        bool createRayTracingInstance(AnthemAccStructInstance** pAsInst, AnthemBottomLevelAccStruct* bottomAs, std::vector<float> transform);
        bool createRayTracingShaderGroup(AnthemRayTracingShaders** pShader, std::vector<std::pair<std::string, AnthemRayTracingShaderType>>& shaderFile);
#endif

        // Swapchain Info
        bool getSwapchainImageExtent(uint32_t* width, uint32_t* height);

        // Descriptor Pool
        bool addSamplerArrayToDescriptor(std::vector<AnthemImageContainer*>& images, AnthemDescriptorPool* descPool,uint32_t bindLoc, uint32_t descId);
        bool addStorageImageArrayToDescriptor(std::vector<AnthemImageContainer*>& images, AnthemDescriptorPool* descPool,uint32_t bindLoc, uint32_t descId);
        bool addUniformBufferArrayToDescriptor(std::vector<AnthemUniformBuffer*>& buffers, AnthemDescriptorPool* descPool, uint32_t bindLoc, uint32_t descId);

        // Integration
        bool registerPipelineSubComponents();

        // Command Buffer Operations
        bool drAllocateCommandBuffer(uint32_t* commandBufferId);
        bool drGetCommandBufferForFrame(uint32_t* commandBufferId,uint32_t frameIdx);
        bool drPrepareFrame(uint32_t currentFrame, uint32_t* avaImageIdx);
        bool drStartCommandRecording(uint32_t cmdIdx);
        bool drEndCommandRecording(uint32_t cmdIdx);
        bool drStartRenderPass(AnthemRenderPass* renderPass,AnthemFramebuffer* framebufferList ,uint32_t cmdIdx,bool enableMsaa);
        bool drEndRenderPass(uint32_t cmdIdx);
        bool drPresentFrame(uint32_t frameIdx, uint32_t avaImageIdx);
        bool drSubmitBufferPrimaryCall(uint32_t frameIdx,uint32_t cmdIdx);
        bool drClearCommands(uint32_t cmdIdx);
        bool drPushConstants(AnthemPushConstant* pushConstant, AnthemGraphicsPipeline* pipeline, uint32_t cmdIdx);

        bool drSubmitCommandBufferGraphicsQueueGeneral(uint32_t cmdIdx, uint32_t frameIdx,
            const std::vector<const AnthemSemaphore*>* semaphoreToWait, const std::vector<AtSyncSemaphoreWaitStage>* semaphoreWaitStages,
            AnthemFence* customFence=nullptr, bool customImageAvailableSemaphore=false);
        bool drSubmitCommandBufferGraphicsQueueGeneral2(uint32_t cmdIdx, uint32_t frameIdx,
            const std::vector<const AnthemSemaphore*>* semaphoreToWait, const std::vector<AtSyncSemaphoreWaitStage>* semaphoreWaitStages,
            AnthemFence* customFence, const std::vector<const AnthemSemaphore*>* semaphoreToSignal);


        bool drSubmitCommandBufferCompQueueGeneral(uint32_t cmdIdx,const std::vector<const AnthemSemaphore*>* semaphoreToWait,const std::vector<const AnthemSemaphore*>* semaphoreToSignal,const AnthemFence* fenceToSignal);
        bool drSetViewportScissorFromSwapchain(uint32_t cmdIdx);
        bool drSetViewportScissor(AnthemViewport* custVp,uint32_t cmdIdx);
        bool drSetLineWidth(float lineWidth, uint32_t cmdIdx);
        bool drBindGraphicsPipeline(AnthemGraphicsPipeline* pipeline,uint32_t cmdIdx);
        bool drBindComputePipeline(AnthemComputePipeline* pipeline,uint32_t cmdIdx);

        bool drBindVertexBuffer(AnthemVertexBuffer* vertexBuffer,uint32_t cmdIdx);
        bool drBindVertexBufferMultiple(std::vector<AnthemVertexBuffer*> vertexBuffer, uint32_t cmdIdx);
        bool drBindVertexBufferFromSsbo(AnthemShaderStorageBuffer* vertexBuffer, uint32_t copyId, uint32_t cmdIdx);
        bool drBindIndexBuffer(AnthemIndexBuffer* indexBuffer,uint32_t cmdIdx);
        bool drBindDescriptorSet(AnthemDescriptorPool* descPool, AnthemGraphicsPipeline* pipeline, uint32_t frameIdx,uint32_t cmdIdx);
        bool drBindDescriptorSetCustomizedGraphics(std::vector<AnthemDescriptorSetEntry> descSetEntries, AnthemGraphicsPipeline* pipeline, uint32_t cmdIdx);
        bool drBindDescriptorSetCustomizedCompute(std::vector<AnthemDescriptorSetEntry> descSetEntries, AnthemComputePipeline* pipeline, uint32_t cmdIdx);
        
        bool drColorImagePipelineBarrier(AnthemImageContainer* container, AnthemImagePipelineBarrier* srcProp, AnthemImagePipelineBarrier* dstProp, uint32_t cmdIdx);
        bool drStorageBufferPipelineBarrier(AnthemShaderStorageBuffer* buffer, uint32_t copyId, AnthemBufferBarrierProp* src, AnthemBufferBarrierProp* dst, uint32_t cmdIdx);

        bool drDraw(uint32_t vertices,uint32_t cmdIdx);
        bool drDrawInstanced(uint32_t vertices, uint32_t instances, uint32_t cmdIdx);
        bool drDrawInstancedAll(uint32_t vertices, uint32_t instances, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance, uint32_t cmdIdx);
        bool drDrawIndexedIndirect(AnthemIndirectDrawBuffer* buffer, uint32_t cmdIdx);
        bool drDrawMesh(uint32_t groupX, uint32_t groupY, uint32_t groupZ, uint32_t cmdIdx);
        bool drComputeDispatch(uint32_t cmdIdx, uint32_t workgroupX, uint32_t workgroupY, uint32_t workgroupZ);
        
#ifdef AT_FEATURE_RAYTRACING_ENABLED
        bool drTraceRays(uint32_t cmdIdx, AnthemRayTracingPipeline* pipeline, uint32_t height, uint32_t width,
            int32_t raygenId,int32_t missId,int32_t closeHitId,int32_t callableId);
#endif

        // Queue
        bool quGetComputeQueueIdx(uint32_t* queue);
        bool quGetGraphicsQueueIdx(uint32_t* queue);


        // Legacy
        bool presentFrameDemo(uint32_t currentFrame, AnthemRenderPass* renderPass,
            AnthemGraphicsPipeline* pipeline, AnthemSwapchainFramebuffer* framebuffer, uint32_t avaImageIdx,
            AnthemVertexBuffer* vbuf, AnthemUniformBuffer* ubuf, AnthemIndexBuffer* ibuf, AnthemDescriptorPool* descPool);
       
        // External 
        bool exGetWindowSize(int& height,int& width);

        // External ImGUI
        static void exImGuiCheckStat(VkResult err){
            ANTH_ASSERT(err == VK_SUCCESS, "exImGuiCheckStat:", err);
        }
        bool exRenderImGui(uint32_t frame, AnthemSwapchainFramebuffer* fb, std::array<float, 4> clearColor, ImDrawData* drawData);
        bool exGetImGuiCommandBufferIndex(uint32_t frame, uint32_t* result);
        bool exGetImGuiDrawProgressSemaphore(uint32_t frame, AnthemSemaphore** result);
        bool exInitImGui();
        bool exDestroyImgui();


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

        template<typename... T, uint32_t... S>
        bool createInstancingBuffer(AnthemInstancingVertexBufferImpl<AnthemVAOAttrDesc<T, S>...>** pInstancingBuffer) {
            auto vxBufferImpl = new AnthemInstancingVertexBufferImpl<AnthemVAOAttrDesc<T, S>...>();
            vxBufferImpl->specifyLogicalDevice(this->logicalDevice.get());
            vxBufferImpl->specifyPhyDevice(this->phyDevice.get());
            vxBufferImpl->specifyCommandBuffers(this->commandBuffers.get());
            *pInstancingBuffer = vxBufferImpl;
            this->vertexBuffers.push_back(vxBufferImpl);
            return true;
        }

        template<typename... T,uint32_t... Rk,uint32_t... Sz,uint32_t... ArrSz>
        bool createUniformBuffer(AnthemUniformBufferImpl<AnthemUBDesc<T,Rk,Sz,ArrSz>...>** pUniformBuffer, uint32_t bindLoc,
            AnthemDescriptorPool *descPool,uint32_t descId=-1){

            //Allocate Uniform Buffer
            auto ubuf = new AnthemUniformBufferImpl<AnthemUBDesc<T,Rk,Sz,ArrSz>...>();
            ubuf->specifyLogicalDevice(this->logicalDevice.get());
            ubuf->specifyPhyDevice(this->phyDevice.get());
            ubuf->createBuffer(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT);

            //Allocate Descriptor Set
            if (descId == -1) {
                ANTH_LOGW("Descriptor pool index not specified for UBO, using the default value", this->uniformDescPoolIdx);
                descId = this->uniformDescPoolIdx;
            }
            if (descPool != nullptr) descPool->addUniformBuffer(ubuf,bindLoc,this->uniformDescPoolIdx);
            *pUniformBuffer = ubuf;
            this->uniformBuffers.push_back(ubuf);
            return true;
        }

        template< template <typename Tp,uint32_t MatDim,uint32_t VecSz,uint32_t ArrSz> class... DescTp, 
            typename... Tp,uint32_t... MatDim,uint32_t... VecSz,uint32_t... ArrSz>
        bool createShaderStorageBuffer(AnthemShaderStorageBufferImpl<DescTp<Tp,MatDim,VecSz,ArrSz>...>** pSsbo,uint32_t totSize,
            uint32_t bindLoc, AnthemDescriptorPool* descPool,
            std::optional<std::function<void(AnthemShaderStorageBufferImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...>*)>> dataPrepareProc,
            uint32_t descId=-1) {

            auto ssbo = new AnthemShaderStorageBufferImpl<DescTp<Tp,MatDim,VecSz,ArrSz>...>();
            ssbo->specifyCommandBuffers(this->commandBuffers.get());
            ssbo->specifyLogicalDevice(this->logicalDevice.get());
            ssbo->specifyPhyDevice(this->phyDevice.get());
            ssbo->specifyNumCopies(this->config->VKCFG_MAX_IMAGES_IN_FLIGHT);
            ssbo->setTotalElements(totSize);
            ssbo->specifyUsage(AnthemSSBOUsage::AT_ASBU_VERTEX);
            if (dataPrepareProc.has_value()) {
                auto w = dataPrepareProc.value();
                w(ssbo);
            }
            *pSsbo = ssbo;
            
            ssbo->createShaderStorageBuffer();
            this->ssboBuffers.push_back(ssbo);

            if (descId == -1) {
                ANTH_LOGW("Descriptor pool index not specified for SSBO, using the default value", this->ssboDescPoolIdx);
                descId = this->ssboDescPoolIdx;
            }
            descPool->addShaderStorageBuffer(ssbo,bindLoc,descId);
            return true;
        }

        template< template <typename Tp, uint32_t MatDim, uint32_t VecSz, uint32_t ArrSz> class... DescTp,
            typename... Tp, uint32_t... MatDim, uint32_t... VecSz, uint32_t... ArrSz>
        bool createPushConstant(AnthemPushConstantImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...>** pPushConst) {
            auto obj = new AnthemPushConstantImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...>();
            pushConstants.push_back(obj);
            *pPushConst = obj;
            return true;
        }
    };
}