#include "../../../include/core/utils/AnthemUtlPhyDeviceReqBase.h"

namespace Anthem::Core::Util{
    bool AnthemUtlPhyDeviceReqBase::specifyPhyDevice(const AnthemPhyDevice* device){
        phyDevice = device;
        return true;
    }
    const AnthemPhyDevice* AnthemUtlPhyDeviceReqBase::getPhyDevice() const{
        return phyDevice;
    }
}