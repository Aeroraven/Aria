#pragma once
#include "../base/AnthemConfig.h"

namespace Anthem::Core::Util{
    class AnthemUtlConfigReqBase{
    protected:
        const AnthemConfig* config = nullptr;
    public:
        bool virtual specifyConfig(const AnthemConfig* config);
        const AnthemConfig* getConfig() const;
    };
}