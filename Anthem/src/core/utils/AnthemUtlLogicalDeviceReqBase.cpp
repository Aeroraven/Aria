#include "../../../include/core/utils/AnthemUtlLogicalDeviceReqBase.h"

namespace Anthem::Core::Util{
    bool AnthemUtlLogicalDeviceReqBase::specifyLogicalDevice(const AnthemLogicalDevice* device){
        logicalDevice = device;
        return true;
    }
    const AnthemLogicalDevice* AnthemUtlLogicalDeviceReqBase::getLogicalDevice() const{
        return logicalDevice;
    }
}