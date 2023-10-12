#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlSwapChainReqBase.h"
#include "../utils/AnthemUtlAppConfigReqBase.h"

namespace Anthem::Core{
    class AnthemMainLoopSyncer:public Util::AnthemUtlLogicalDeviceReqBase,
        public Util::AnthemUtlSwapChainReqBase,public Util::AnthemUtlConfigReqBase{
    private:
        std::vector<VkFence> inFlightFence = {};
        std::vector<VkSemaphore> imageAvailableSp = {};
        std::vector<VkSemaphore> drawFinishedSp = {};

        bool syncObjectAvailable = false;
    public:
        bool createSyncObjects();
        bool destroySyncObjects();

        bool waitForPrevFrame(uint32_t frameIdx);
        bool resetFence(uint32_t frameIdx);
        uint32_t acquireNextFrame(uint32_t frameIdx,std::function<void()> swapChainOutdatedHandler = ANTH_ERROR_RAISE_DEFAULT_FUNC);
        bool submitCommandBuffer(const VkCommandBuffer* commandBuffer,uint32_t frameIdx);
        bool presentFrame(uint32_t imageIndex,uint32_t frameIdx,std::function<void()> swapChainOutdatedHandler = ANTH_ERROR_RAISE_DEFAULT_FUNC);
    };
}