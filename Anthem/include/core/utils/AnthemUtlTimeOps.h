#pragma once
#include "../base/AnthemBaseImports.h"

namespace Anthem::Core::Util{
    class AnthemUtlTimeOps{
    protected:
        decltype(std::chrono::steady_clock::now()) lastTimePoint;
    public:
        int64_t getRelativeTimePoint(){
            const auto curRefTime = std::chrono::steady_clock::now();
            const auto t = std::chrono::duration_cast<std::chrono::milliseconds>(curRefTime.time_since_epoch());
            return t.count();
        }
        bool recordTime(){
            lastTimePoint = std::chrono::steady_clock::now();
        }
        int64_t getDuration(){
            auto duration = std::chrono::steady_clock::now() - lastTimePoint;
            auto mili = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            return mili.count();
        }
    };
}