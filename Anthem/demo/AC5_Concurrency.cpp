#include "../include/core/base/AnthemBaseImports.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include <atomic>
#include <initializer_list>

using namespace Anthem::Core;

static int exampleCounter = 0;
#define BEGIN_EXAMPLE(x) exampleCounter+=1;ANTH_LOGI("Example ",exampleCounter,": ",x,"=====================");{
#define END_EXAMPLE() } ANTH_LOGI("=========================================");

#define JUDGE_EXPRESSION(x) if((x)){ANTH_LOGI("Statement is TRUE: ", #x);}else{ANTH_LOGI("Statement is FALSE: ", #x);}
#define RUN_EXPRESSION(x) ANTH_LOGI("Running:", #x);x;
#define EVAL_EXPRESSION(x) ANTH_LOGI("Value of ",#x," is ",(x));
#define PRINT(...) ANTH_LOGI(__VA_ARGS__);
#define TRACE_TIME(x) auto x = std::chrono::steady_clock().now();
#define PRINT_TIME(x) PRINT("Timer ",#x,":",std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock().now()-(x)).count()," ms");
namespace Anthem::AcStage::Concurrency {

    void run() {
        BEGIN_EXAMPLE("Atomic I");
            TRACE_TIME(x);
            int cnt = 0;
            std::atomic<int> atomCnt = 0;
            int atomCntW = 0;
            auto func = [&]() {
                for (auto i : AT_RANGE2(10000)) {
                    cnt++;
                    atomCnt++;
                }
            };
            std::vector<std::thread> pool;
            for (auto i : AT_RANGE2(50))pool.push_back(std::thread(func));
            for (auto i : AT_RANGE2(50))pool[i].join();
            EVAL_EXPRESSION(cnt);
            atomCntW = atomCnt;
            EVAL_EXPRESSION(atomCntW);
            PRINT_TIME(x);
        END_EXAMPLE();

        BEGIN_EXAMPLE("Atomic II");
            TRACE_TIME(x);
            int cnt = 0;
            std::atomic<int> atomCnt = 0;
            int atomCntW = 0;
            std::mutex lock;
            auto func = [&]() {
                for (auto i : AT_RANGE2(100000)) {
                    lock.lock();
                    cnt++;
                    lock.unlock();
                    atomCnt++;
                }
                };
            std::vector<std::thread> pool;
            for (auto i : AT_RANGE2(50))pool.push_back(std::thread(func));
            for (auto i : AT_RANGE2(50))pool[i].join();
            EVAL_EXPRESSION(cnt);
            atomCntW = atomCnt;
            EVAL_EXPRESSION(atomCntW);
            PRINT_TIME(x);
        END_EXAMPLE();

        BEGIN_EXAMPLE("Lock Guard I");
            std::timed_mutex mutex;
            int cnt = 0;
            int w = 0;
            auto luckyFunc = [&]() {
                using namespace std::chrono_literals;
                bool rp = mutex.try_lock_for(+2s);
                if (rp == false)return;
                if ((w++) % 2 == 0)throw std::runtime_error("Lucky");
                for (auto i : AT_RANGE2(100000)) {
                    cnt++;
                }
                mutex.unlock();
            };
            auto threadWrapper = [&]() {
                try {
                    luckyFunc();
                }catch (std::exception e) {
                    PRINT("I Am Lucky");
                }
            };
            std::vector<std::thread> pool;
            for (auto i : AT_RANGE2(10))pool.push_back(std::thread(threadWrapper));
            for (auto i : AT_RANGE2(10)) {
                PRINT("Thread ", i, " joins");
                pool[i].join();
            }
        END_EXAMPLE();

        BEGIN_EXAMPLE("Lock Guard II");
            std::timed_mutex mutex;
            int cnt = 0;
            int w = 0;
            auto luckyFunc = [&]() {
                using namespace std::chrono_literals;
                std::lock_guard<std::timed_mutex> lock(mutex);
                if ((w++) % 2 == 0)throw std::runtime_error("Lucky");
                for (auto i : AT_RANGE2(100000)) {
                    cnt++;
                }
                };
            auto threadWrapper = [&]() {
                try {
                    luckyFunc();
                }
                catch (std::exception e) {
                    PRINT("I Am Lucky");
                }
                };
            std::vector<std::thread> pool;
            for (auto i : AT_RANGE2(10))pool.push_back(std::thread(threadWrapper));
            for (auto i : AT_RANGE2(10)) {
                PRINT("Thread ", i, " joins");
                pool[i].join();
            }
        END_EXAMPLE();
    }
}

int main() {
    Anthem::AcStage::Concurrency::run();
}