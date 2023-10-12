#include "../../../include/core/drawing/AnthemMainLoopSyncer.h"

namespace Anthem::Core{
    bool AnthemMainLoopSyncer::createSyncObjects(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto inFlightCreatRes = vkCreateFence(this->logicalDevice->getLogicalDevice(),&fenceInfo,nullptr,&this->inFlightFence);
        auto imageAvailableCreatRes = vkCreateSemaphore(this->logicalDevice->getLogicalDevice(),&semaphoreInfo,nullptr,&this->imageAvailableSp);
        auto drawFinishedCreatRes = vkCreateSemaphore(this->logicalDevice->getLogicalDevice(),&semaphoreInfo,nullptr,&this->drawFinishedSp);

        if(inFlightCreatRes != VK_SUCCESS || imageAvailableCreatRes != VK_SUCCESS || drawFinishedCreatRes != VK_SUCCESS){
            ANTH_LOGE("Failed to create sync objects",inFlightCreatRes,imageAvailableCreatRes,drawFinishedCreatRes);
            return false;
        }
        this->syncObjectAvailable = true;
        ANTH_LOGI("Sync objects created");
        return true;

    }
    bool AnthemMainLoopSyncer::destroySyncObjects(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->syncObjectAvailable,"Sync objects not created");
        ANTH_LOGI("Destroying sync objects");
        vkDestroySemaphore(this->logicalDevice->getLogicalDevice(),this->imageAvailableSp,nullptr);
        vkDestroySemaphore(this->logicalDevice->getLogicalDevice(),this->drawFinishedSp,nullptr);
        vkDestroyFence(this->logicalDevice->getLogicalDevice(),this->inFlightFence,nullptr);
        return true;
    }
    bool AnthemMainLoopSyncer::waitForPrevFrame(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->syncObjectAvailable,"Sync objects not created");
        ANTH_LOGI("Waiting for fences");
        vkWaitForFences(this->logicalDevice->getLogicalDevice(),1,&this->inFlightFence,VK_TRUE,UINT64_MAX);
        vkResetFences(this->logicalDevice->getLogicalDevice(),1,&this->inFlightFence);
        return true;
    }
    uint32_t AnthemMainLoopSyncer::acquireNextFrame(){
        ANTH_ASSERT(this->logicalDevice != nullptr,"Logical device not specified");
        ANTH_ASSERT(this->syncObjectAvailable,"Sync objects not created");
        ANTH_LOGI("Acquiring next frame");
        uint32_t imageIndex;
        vkAcquireNextImageKHR(this->logicalDevice->getLogicalDevice(),*(this->swapChain->getSwapChain()),UINT64_MAX,this->imageAvailableSp,VK_NULL_HANDLE,&imageIndex);
        return imageIndex;
    }
    bool AnthemMainLoopSyncer::submitCommandBuffer(const VkCommandBuffer* commandBuffer){
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {
            this->imageAvailableSp
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
            this->drawFinishedSp
        };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if(vkQueueSubmit(this->logicalDevice->getGraphicsQueue(),1,&submitInfo,this->inFlightFence)!=VK_SUCCESS){
            ANTH_LOGE("Failed to submit draw command buffer");
            return false;
        }
        ANTH_LOGI("Draw command buffer submitted");
        return true;
    }
    bool AnthemMainLoopSyncer::presentFrame(uint32_t imageIndex){
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        VkSemaphore waitSemaphores[] = {
            this->drawFinishedSp
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
            ANTH_LOGE("Swap chain out of date or suboptimal");
            return false;
        }else if(result != VK_SUCCESS){
            ANTH_LOGE("Failed to present swap chain image",result);
            return false;
        }
        ANTH_LOGI("Swap chain image presented");
        return true;
    }
}