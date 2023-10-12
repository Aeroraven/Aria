#include "../../../include/core/utils/AnthemUtlAppConfigReqBase.h"

namespace Anthem::Core::Util{
    bool AnthemUtlConfigReqBase::specifyConfig(const AnthemConfig* config){
        this->config = config;
        return true;
    }
    const AnthemConfig* AnthemUtlConfigReqBase::getConfig() const{
        return config;
    }
}