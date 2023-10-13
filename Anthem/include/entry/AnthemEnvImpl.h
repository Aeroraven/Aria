#pragma once
#include "../core/base/AnthemEnv.h"
#include "../core/base/AnthemDefs.h"
#include "../core/base/AnthemConfig.h"
#include "../core/base/AnthemLogger.h"
#include "../core/base/AnthemValLayer.h"
#include "../core/base/AnthemPhyDeviceSelector.h"
#include "../core/base/AnthemLogicalDeviceSelector.h"
#include "../core/base/AnthemWindowSurface.h"
#include "../core/base/AnthemSwapChain.h"
#include "../core/base/AnthemInstance.h"
#include "../core/pipeline/AnthemShaderModule.h"
#include "../core/pipeline/AnthemGraphicsPipeline.h"
#include "../core/pipeline/AnthemRenderPass.h"
#include "../core/drawing/AnthemFramebufferList.h"
#include "../core/drawing/AnthemCommandManager.h"
#include "../core/drawing/AnthemMainLoopSyncer.h"
#include "../core/drawing/AnthemVertexBuffer.h"

namespace Anthem::Entry{
    using namespace Anthem::Core;
    
    class AnthemEnvImpl: public AnthemEnv {
    private: //Member Variables
        ANTH_SHARED_PTR(AnthemInstance) instance;
        
        ANTH_SHARED_PTR(AnthemConfig) cfg;
        ANTH_SHARED_PTR(AnthemValLayer) valLayer;
        ANTH_SHARED_PTR(AnthemPhyDeviceSelector) phyDeviceSelector;
        ANTH_SHARED_PTR(AnthemLogicalDeviceSelector) logicalDeviceSelector;
        ANTH_SHARED_PTR(AnthemWindowSurface) windowSurface;

        ANTH_SHARED_PTR(AnthemSwapChain) swapChain;
        ANTH_SHARED_PTR(AnthemPhyDevice) phyDevice;
        ANTH_SHARED_PTR(AnthemLogicalDevice) logicalDevice;

        ANTH_SHARED_PTR(AnthemShaderModule) shader;
        ANTH_SHARED_PTR(AnthemGraphicsPipeline) graphicsPipeline;
        ANTH_SHARED_PTR(AnthemViewport) viewport;
        ANTH_SHARED_PTR(AnthemRenderPass) renderPass;

        ANTH_SHARED_PTR(AnthemFramebufferList) framebufferList;
        ANTH_SHARED_PTR(AnthemCommandManager) commandManager;
        ANTH_SHARED_PTR(AnthemMainLoopSyncer) mainLoopSyncer;

        AnthemVertexBuffer* vertexBuffer;

        int currentFrame = 0;
        bool resizeStatusFlag = false;

    public: //Member Functions
        AnthemEnvImpl(ANTH_SHARED_PTR(AnthemConfig) cfg);
        bool virtual createWindow() override;
        void virtual drawLoop() override;
        void virtual createInstance() override;
        void virtual destroyEnv() override;
        void virtual run() override; 
        void virtual init() override;

        void virtual initSwapChain();
        void virtual loadShader();
        void virtual initGraphicsPipeline();
        void virtual initRenderPass();

        void virtual initFramebuffer();
        void virtual initCommandManager();
        void virtual initSyncObjects();
        void virtual drawFrame();

        void virtual destroySwapChain();
        void virtual recreateSwapChain();

        void virtual createVertexBuffer();
    };
}


