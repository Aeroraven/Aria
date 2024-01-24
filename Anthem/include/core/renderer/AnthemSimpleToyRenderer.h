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


        std::vector<AnthemRenderPass*> renderPasses;
        std::vector<AnthemImage*> textures;
        std::vector<AnthemShaderModule*> shaders;

        std::vector<AnthemVertexBuffer*> vertexBuffers;
        std::vector<AnthemIndexBuffer*> indexBuffers;
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


        uint32_t uniformDescPoolIdx = -1;
        uint32_t imageDescPoolIdx = -1;
        uint32_t ssboDescPoolIdx = -1;

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

        bool setConfig(const AnthemConfig* config);
        bool initialize();
        bool finalize();

        bool startDrawLoopDemo();
        bool setDrawFunction(std::function<void()> drawLoopHandler);

        bool presentFrameDemo(uint32_t currentFrame, AnthemRenderPass* renderPass, 
            AnthemGraphicsPipeline* pipeline, AnthemSwapchainFramebuffer* framebuffer,uint32_t avaImageIdx,
            AnthemVertexBuffer* vbuf, AnthemUniformBuffer* ubuf,AnthemIndexBuffer* ibuf, AnthemDescriptorPool* descPool);

        bool setupDemoRenderPass(AnthemRenderPass** pRenderPass, AnthemDepthBuffer* depthBuffer, bool retain=false);
        bool setupRenderPass(AnthemRenderPass** pRenderPass, AnthemRenderPassSetupOption* setupOption, AnthemDepthBuffer* depthBuffer);
        bool createDepthBuffer(AnthemDepthBuffer** pDepthBuffer, bool enableMsaa);
        bool createDepthBufferWithSampler(AnthemDepthBuffer** pDepthBuffer,AnthemDescriptorPool* descPool, uint32_t bindLoc, bool enableMsaa);
        bool createTexture(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth,uint32_t texHeight,
             uint32_t texChannel, uint32_t bindLoc, bool generateMipmap2D, bool enableMsaa,AnthemImageFormat imageFmt = AnthemImageFormat::AT_IF_SRGB_UINT8);
        bool createTexture3d(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint8_t* texData, uint32_t texWidth, uint32_t texHeight, uint32_t texDepth,
            uint32_t texChannel, uint32_t bindLoc, AnthemImageFormat imageFmt = AnthemImageFormat::AT_IF_SRGB_UINT8);
        bool createColorAttachmentImage(AnthemImage** pImage, AnthemDescriptorPool* descPool, uint32_t bindLoc,  AnthemImageFormat format, bool enableMsaa);
        bool createIndexBuffer(AnthemIndexBuffer** pIndexBuffer);
        bool createShader(AnthemShaderModule** pShaderModule,AnthemShaderFilePaths* filename);
        bool createSwapchainImageFramebuffers(AnthemSwapchainFramebuffer** pFramebufferList,const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer);
        bool createSimpleFramebuffer(AnthemFramebuffer** pFramebuffer, const std::vector<const AnthemImage*>* colorAttachment, const AnthemRenderPass* renderPass, const AnthemDepthBuffer* depthBuffer);
        bool createDescriptorPool(AnthemDescriptorPool** pDescriptorPool);

        bool registerPipelineSubComponents();
        bool createGraphicsPipeline(AnthemGraphicsPipeline** pPipeline,  AnthemDescriptorPool* descPool, AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule, IAnthemVertexBufferAttrLayout* vertexBuffer,AnthemUniformBuffer* uniformBuffer);
        bool createGraphicsPipelineCustomized(AnthemGraphicsPipeline** pPipeline,std::vector<AnthemDescriptorSetEntry> descSetEntries,AnthemRenderPass* renderPass,AnthemShaderModule* shaderModule, IAnthemVertexBufferAttrLayout* vertexBuffer,AnthemGraphicsPipelineCreateProps* createProps);
        bool createComputePipelineCustomized(AnthemComputePipeline** pPipeline,std::vector<AnthemDescriptorSetEntry> descSetEntries,AnthemShaderModule* shaderModule);
        
        bool createSemaphore(AnthemSemaphore** pSemaphore);
        bool createFence(AnthemFence** pFence);

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

        bool drSubmitCommandBufferGraphicsQueueGeneral(uint32_t cmdIdx, uint32_t frameIdx,
            const std::vector<const AnthemSemaphore*>* semaphoreToWait, const std::vector<AtSyncSemaphoreWaitStage>* semaphoreWaitStages,
            AnthemFence* customFence=nullptr, bool customImageAvailableSemaphore=false);
        bool drSubmitCommandBufferGraphicsQueueGeneral2(uint32_t cmdIdx, uint32_t frameIdx,
            const std::vector<const AnthemSemaphore*>* semaphoreToWait, const std::vector<AtSyncSemaphoreWaitStage>* semaphoreWaitStages,
            AnthemFence* customFence, const std::vector<const AnthemSemaphore*>* semaphoreToSignal);


        bool drSubmitCommandBufferCompQueueGeneral(uint32_t cmdIdx,const std::vector<const AnthemSemaphore*>* semaphoreToWait,const std::vector<const AnthemSemaphore*>* semaphoreToSignal,const AnthemFence* fenceToSignal);

        bool drSetViewportScissor(uint32_t cmdIdx);
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
        bool drDraw(uint32_t vertices,uint32_t cmdIdx);
        bool drDrawInstanced(uint32_t vertices, uint32_t instances, uint32_t cmdIdx);
        bool drComputeDispatch(uint32_t cmdIdx, uint32_t workgroupX, uint32_t workgroupY, uint32_t workgroupZ);
        
        bool exGetWindowSize(int& height,int& width);

        static void exImGuiCheckStat(VkResult err)
        {
            if (err == 0)
                return;
            fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
            if (err < 0)
                abort();
        }
        bool exRenderImGui(uint32_t frame,AnthemSwapchainFramebuffer* fb, std::array<float,4> clearColor, ImDrawData* drawData) {
            VkClearValue clearValue[2];
            clearValue[0].color = { {clearColor[0], clearColor[1], clearColor[2], clearColor[3]} };
            clearValue[1].depthStencil = { 1.0f, 0 };

            this->commandBuffers->resetCommandBuffer(this->g_CommandBufferIdx[frame]);
            this->drStartCommandRecording(this->g_CommandBufferIdx[frame]);
            {
                VkRenderPassBeginInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                info.renderPass = this->g_RenderPass;
                info.framebuffer = *fb->getFramebufferObject(frame)->getFramebuffer();
                info.renderArea.extent.width = this->swapChain->getSwapChainExtentWidth();
                info.renderArea.extent.height = this->swapChain->getSwapChainExtentHeight();
                info.clearValueCount = 2;
                info.pClearValues = clearValue;
                vkCmdBeginRenderPass(*this->commandBuffers->getCommandBuffer(this->g_CommandBufferIdx[frame]), &info, VK_SUBPASS_CONTENTS_INLINE);
            }
            ImGui_ImplVulkan_RenderDrawData(drawData, *this->commandBuffers->getCommandBuffer(this->g_CommandBufferIdx[frame]));
            this->drEndRenderPass(this->g_CommandBufferIdx[frame]);
 
            this->drEndCommandRecording(this->g_CommandBufferIdx[frame]);
            return true;
        }

        bool exGetImGuiCommandBufferIndex(uint32_t frame, uint32_t* result) {
            *result = this->g_CommandBufferIdx[frame];
            return true;
        }
        bool exGetImGuiDrawProgressSemaphore(uint32_t frame, AnthemSemaphore** result) {
            *result = this->g_RenderComplete[frame];
            return true;
        }

        bool exInitImGui() {
            {
                VkDescriptorPoolSize pool_sizes[] =
                {
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
                };
                VkDescriptorPoolCreateInfo pool_info = {};
                pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
                pool_info.maxSets = 1;
                pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
                pool_info.pPoolSizes = pool_sizes;
                auto err = vkCreateDescriptorPool(this->logicalDevice->getLogicalDevice(), &pool_info, nullptr, &g_DescriptorPool);
                exImGuiCheckStat(err);
            }
            {
                VkAttachmentDescription attachment = {};
                attachment.format = *this->swapChain->getFormat();
                attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                VkAttachmentReference color_attachment = {};
                color_attachment.attachment = 0;
                color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                VkAttachmentDescription depthAttachment = {};
                VkAttachmentReference depthAttachmentRef{};
                depthAttachment.format = VK_FORMAT_D32_SFLOAT;
                depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthAttachmentRef.attachment = 1;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                VkSubpassDescription subpass = {};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments = &color_attachment;
                subpass.pDepthStencilAttachment = &depthAttachmentRef;

                VkSubpassDependency dependency = {};
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.srcAccessMask = 0;
                dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;


                std::vector<VkAttachmentDescription> desc = { attachment,depthAttachment };
                VkRenderPassCreateInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                info.attachmentCount = 2;
                info.pAttachments = desc.data();
                info.subpassCount = 1;
                info.pSubpasses = &subpass;
                info.dependencyCount = 1;
                info.pDependencies = &dependency;
                auto err = vkCreateRenderPass(this->logicalDevice->getLogicalDevice(), &info, nullptr, &this->g_RenderPass);
                exImGuiCheckStat(err);

            }
            ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)this->instance->getWindow(), true);
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = *this->instance->getInstance();
            init_info.PhysicalDevice = this->phyDevice->getPhysicalDevice();
            init_info.Device = this->logicalDevice->getLogicalDevice();
            init_info.QueueFamily = this->phyDevice->getPhyQueueGraphicsFamilyIndice().value();
            init_info.Queue = this->logicalDevice->getComputeQueue();
            init_info.PipelineCache = VK_NULL_HANDLE;
            init_info.DescriptorPool = g_DescriptorPool;
            init_info.Subpass = 0;
            init_info.MinImageCount = 2;
            init_info.ImageCount = 2;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = nullptr;
            init_info.CheckVkResultFn = exImGuiCheckStat;
            ImGui_ImplVulkan_Init(&init_info, g_RenderPass);

            // Sync & Rd
            this->g_CommandBufferIdx = new uint32_t[this->config->VKCFG_MAX_IMAGES_IN_FLIGHT];
            this->g_RenderComplete = new AnthemSemaphore*[this->config->VKCFG_MAX_IMAGES_IN_FLIGHT];
            for (int i = 0; i < this->config->VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
                this->drAllocateCommandBuffer(&this->g_CommandBufferIdx[i]);
                this->createSemaphore(&this->g_RenderComplete[i]);
            }
            return true;
        }

        bool exDestroyImgui() {
            ImGui_ImplVulkan_Shutdown();
            vkDestroyRenderPass(this->logicalDevice->getLogicalDevice(), g_RenderPass, nullptr);
            vkDestroyDescriptorPool(this->logicalDevice->getLogicalDevice(), g_DescriptorPool, nullptr);
            return true;
        }

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

        template< template <typename Tp,uint32_t MatDim,uint32_t VecSz,uint32_t ArrSz> class... DescTp, 
            typename... Tp,uint32_t... MatDim,uint32_t... VecSz,uint32_t... ArrSz>
        bool createShaderStorageBuffer(AnthemShaderStorageBufferImpl<DescTp<Tp,MatDim,VecSz,ArrSz>...>** pSsbo,uint32_t totSize, uint32_t bindLoc, AnthemDescriptorPool* descPool,
            std::optional<std::function<void(AnthemShaderStorageBufferImpl<DescTp<Tp, MatDim, VecSz, ArrSz>...>*)>> dataPrepareProc) {
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
            descPool->addShaderStorageBuffer(ssbo,bindLoc,this->ssboDescPoolIdx);
            return true;
        }

    };
}