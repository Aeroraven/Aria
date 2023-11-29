#include "../include/core/base/AnthemBaseImports.h"
#include <initializer_list>
using namespace Anthem::Core;

static int exampleCounter = 0;
#define BEGIN_EXAMPLE(x) exampleCounter+=1;ANTH_LOGI("Example ",exampleCounter,": ",x,"=====================");{
#define END_EXAMPLE() } ANTH_LOGI("=========================================");

#define JUDGE_EXPRESSION(x) if((x)){ANTH_LOGI("Statement is TRUE: ", #x);}else{ANTH_LOGI("Statement is FALSE: ", #x);}
#define RUN_EXPRESSION(x) ANTH_LOGI("Running:", #x);x;
#define EVAL_EXPRESSION(x) ANTH_LOGI("Value of ",#x," is ",(x));
#define PRINT(...) ANTH_LOGI(__VA_ARGS__);

namespace Anthem::AcStage::Runtime{
    void run(){
        BEGIN_EXAMPLE("Generic lambda");
            auto foo = []<typename T,typename U>(T a,U b)->decltype(a+b){
                return a+b;
            };
            EVAL_EXPRESSION(foo(1,2));
        END_EXAMPLE();
    }   
}

int main(){
    Anthem::AcStage::Runtime::run();
}