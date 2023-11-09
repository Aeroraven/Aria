#include "../../../../include/core/drawing/synchronization/AnthemFence.h"

namespace Anthem::Core{
    bool AnthemFence::createFence(){
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        auto result = vkCreateFence(this->logicalDevice->getLogicalDevice(),&fenceInfo,nullptr,&this->fence);
        if(result!=VK_SUCCESS){
            ANTH_LOGE("Cannot create fence");
            return false;
        }
        return true;
    }

    const VkFence* AnthemFence::getFence() const{
        return &this->fence;
    }

    bool AnthemFence::waitForFence(){
        auto result = vkWaitForFences(this->logicalDevice->getLogicalDevice(),1,&this->fence,1,UINT64_MAX);
        if(result!=VK_SUCCESS){
            ANTH_LOGE("Cannot wait for fence");
            return false;
        }
        return true;
    }

    bool AnthemFence::resetFence(){
        auto result = vkResetFences(this->logicalDevice->getLogicalDevice(),1,&this->fence);
        if(result!=VK_SUCCESS){
            ANTH_LOGE("Cannot wait for fence");
            return false;
        }
        return true;
    }
}