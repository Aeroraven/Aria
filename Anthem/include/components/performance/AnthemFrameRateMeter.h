#pragma once
#include "../base/AnthemComponentsBaseImports.h"
#include "../../core/utils/AnthemUtlTimeOps.h"

namespace Anthem::Components::Performance{
    using namespace Anthem::Core::Util;
    class AnthemFrameRateMeter:public virtual AnthemUtlTimeOps{
    private:
        int64_t maxRecord = 0;
        int64_t accumulatedTime = 0;
        std::deque<int64_t> timeRecords;
    public:
        AnthemFrameRateMeter(int64_t recordSize);
        void record();
        double getFrameRate();
    };
}