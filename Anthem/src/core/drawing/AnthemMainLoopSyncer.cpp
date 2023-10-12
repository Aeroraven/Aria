#include "../../../include/core/drawing/AnthemMainLoopSyncer.h"

namespace Anthem::Core{
    bool AnthemMainLoopSyncer::createSyncObjects(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->config != nullptr,"Config not specified");

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(auto i=0;i<this->config->VKCFG_MAX_IMAGES_IN_FLIGHT;i++){
            this->inFlightFence.push_back(VK_NULL_HANDLE);
            this->imageAvailableSp.push_back(VK_NULL_HANDLE);
            this->drawFinishedSp.push_back(VK_NULL_HANDLE);
        }

        for(auto i=0;i<this->config->VKCFG_MAX_IMAGES_IN_FLIGHT;i++){
            auto inFlightCreatRes = vkCreateFence(this->logicalDevice->getLogicalDevice(),&fenceInfo,nullptr,&this->inFlightFence.at(i));
            auto imageAvailableCreatRes = vkCreateSemaphore(this->logicalDevice->getLogicalDevice(),&semaphoreInfo,nullptr,&this->imageAvailableSp.at(i));
            auto drawFinishedCreatRes = vkCreateSemaphore(this->logicalDevice->getLogicalDevice(),&semaphoreInfo,nullptr,&this->drawFinishedSp.at(i));

            if(inFlightCreatRes != VK_SUCCESS || imageAvailableCreatRes != VK_SUCCESS || drawFinishedCreatRes != VK_SUCCESS){
                ANTH_LOGE("Failed to create sync objects",inFlightCreatRes,imageAvailableCreatRes,drawFinishedCreatRes);
                return false;
            }
            this->syncObjectAvailable = true;
            ANTH_LOGI("Sync objects created, Idx=",i);
        }
        
        return true;

    }
    bool AnthemMainLoopSyncer::destroySyncObjects(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->syncObjectAvailable,"Sync objects not created");
        ANTH_LOGI("Destroying sync objects");
        for(auto i=0;i<this->config->VKCFG_MAX_IMAGES_IN_FLIGHT;i++){
            vkDestroySemaphore(this->logicalDevice->getLogicalDevice(),this->imageAvailableSp.at(i),nullptr);
            vkDestroySemaphore(this->logicalDevice->getLogicalDevice(),this->drawFinishedSp.at(i),nullptr);
            vkDestroyFence(this->logicalDevice->getLogicalDevice(),this->inFlightFence.at(i),nullptr);
        }

        return true;
    }
    bool AnthemMainLoopSyncer::waitForPrevFrame(uint32_t frameIdx){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->syncObjectAvailable,"Sync objects not created");
        ANTH_LOGV("Waiting for fences, Idx=",frameIdx);
        vkWaitForFences(this->logicalDevice->getLogicalDevice(),1,&this->inFlightFence[frameIdx],VK_TRUE,UINT64_MAX);
        vkResetFences(this->logicalDevice->getLogicalDevice(),1,&this->inFlightFence[frameIdx]);
        return true;
    }
    bool AnthemMainLoopSyncer::resetFence(uint32_t frameIdx){
        vkResetFences(this->logicalDevice->getLogicalDevice(),1,&this->inFlightFence[frameIdx]);
        return true;
    }
    uint32_t AnthemMainLoopSyncer::acquireNextFrame(uint32_t frameIdx,std::function<void()> swapChainOutdatedHandler){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->syncObjectAvailable,"Sync objects not created");
        ANTH_LOGV("Acquiring next frame");
        uint32_t imageIndex;
        auto result = vkAcquireNextImageKHR(this->logicalDevice->getLogicalDevice(),*(this->swapChain->getSwapChain()),UINT64_MAX,this->imageAvailableSp[frameIdx],VK_NULL_HANDLE,&imageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR){
            ANTH_LOGW("Swap chain out of date");
            swapChainOutdatedHandler();
            return UINT32_MAX;
        }else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            ANTH_LOGE("Failed to acquire next image", result);
            return UINT32_MAX-1;
        }
        return imageIndex;
    }
    bool AnthemMainLoopSyncer::submitCommandBuffer(const VkCommandBuffer* commandBuffer,uint32_t frameIdx){
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {
            this->imageAvailableSp[frameIdx]
        };
        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = commandBuffer;
        
        VkSemaphore signalSemaphores[] = {
            this->drawFinishedSp[frameIdx]
        };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if(vkQueueSubmit(this->logicalDevice->getGraphicsQueue(),1,&submitInfo,this->inFlightFence[frameIdx])!=VK_SUCCESS){
            ANTH_LOGE("Failed to submit draw command buffer");
            return false;
        }
        ANTH_LOGV("Draw command buffer submitted");
        return true;
    }
    bool AnthemMainLoopSyncer::presentFrame(uint32_t imageIndex,uint32_t frameIdx,std::function<void()> swapChainOutdatedHandler){
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        VkSemaphore waitSemaphores[] = {
            this->drawFinishedSp[frameIdx]
        };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphores;
        VkSwapchainKHR swapChains[] = {
            *(this->swapChain->getSwapChain())
        };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;
        auto result = vkQueuePresentKHR(this->logicalDevice->getPresentQueue(),&presentInfo);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR){
            ANTH_LOGI("Swap chain out of date or suboptimal");
            swapChainOutdatedHandler();
            return false;
        }else if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to present swap chain image",result);
            return false;
        }
        ANTH_LOGV("Swap chain image presented");
        return true;
    }
}