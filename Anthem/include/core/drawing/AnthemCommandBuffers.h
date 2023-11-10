#pragma once
#include "../base/AnthemBaseImports.h"
#include "../utils/AnthemUtlLogicalDeviceReqBase.h"
#include "../utils/AnthemUtlPhyDeviceReqBase.h"
#include "../utils/AnthemUtlSwapChainReqBase.h"
#include "../utils/AnthemUtlAppConfigReqBase.h"

namespace Anthem::Core{
    class AnthemCommandBuffers: public Util::AnthemUtlLogicalDeviceReqBase, 
        public Util::AnthemUtlPhyDeviceReqBase, public Util::AnthemUtlSwapChainReqBase,
        public Util::AnthemUtlConfigReqBase{
    private:
        VkCommandPoolCreateInfo poolCreateInfo = {};
        VkCommandPool commandPool = nullptr;

        std::vector<VkCommandBuffer*> commandBuffer;

        bool commandPoolCreated = false;
        bool commandBufferStarted = false;
        bool renderPassStarted = false;
    public:
        bool virtual createCommandPool();    
        bool virtual destroyCommandPool();

        bool virtual createCommandBuffer(uint32_t* bufIdx);
        bool virtual resetCommandBuffer(uint32_t bufIdx);
        bool virtual freeCommandBuffer(uint32_t bufIdx);

        bool virtual startCommandRecording(uint32_t bufIdx);
        bool virtual endCommandRecording(uint32_t bufIdx);

        const VkCommandBuffer* getCommandBuffer(uint32_t bufIdx) const;
        bool virtual submitTaskToGraphicsQueue(uint32_t bufIdx, bool waitingForIdle);
    };
}