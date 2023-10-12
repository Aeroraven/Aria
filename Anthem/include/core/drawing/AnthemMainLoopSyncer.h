#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlSwapChainReqBase.h"

namespace Anthem::Core{
    class AnthemMainLoopSyncer:public Util::AnthemUtlLogicalDeviceReqBase,public Util::AnthemUtlSwapChainReqBase{
    private:
        VkFence inFlightFence = VK_NULL_HANDLE;
        VkSemaphore imageAvailableSp = VK_NULL_HANDLE;
        VkSemaphore drawFinishedSp = VK_NULL_HANDLE;

        bool syncObjectAvailable = false;
    public:
        bool createSyncObjects();
        bool destroySyncObjects();

        bool waitForPrevFrame();
        uint32_t acquireNextFrame();
        bool submitCommandBuffer(const VkCommandBuffer* commandBuffer);
        bool presentFrame(uint32_t imageIndex);
    };
}