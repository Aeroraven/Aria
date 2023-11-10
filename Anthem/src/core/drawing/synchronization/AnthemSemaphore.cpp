#include "../../../../include/core/drawing/synchronization/AnthemSemaphore.h"

namespace Anthem::Core{
    bool AnthemSemaphore::createSemaphore(){
        VkSemaphoreCreateInfo sInfo{};
        sInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        auto result = vkCreateSemaphore(this->logicalDevice->getLogicalDevice(),&sInfo,nullptr,&this->semaphore);
        if(result!=VK_SUCCESS){
            ANTH_LOGE("Cannot create fence");
            return false;
        }
        return true;
    }
    const VkSemaphore* AnthemSemaphore::getSemaphore() const{
        return &this->semaphore;
    }
    bool AnthemSemaphore::destroySemaphore() {
        vkDestroySemaphore(this->logicalDevice->getLogicalDevice(), this->semaphore, nullptr);
        return true;
    }
}