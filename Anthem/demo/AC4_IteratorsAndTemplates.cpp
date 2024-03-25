#include "../include/core/base/AnthemBaseImports.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/components/math/AnthemLowDiscrepancySequence.h"
#include <initializer_list>

using namespace Anthem::Core;
using namespace Anthem::Components::Math;

static int exampleCounter = 0;
#define BEGIN_EXAMPLE(x) exampleCounter+=1;ANTH_LOGI("Example ",exampleCounter,": ",x,"=====================");{
#define END_EXAMPLE() } ANTH_LOGI("=========================================");

#define JUDGE_EXPRESSION(x) if((x)){ANTH_LOGI("Statement is TRUE: ", #x);}else{ANTH_LOGI("Statement is FALSE: ", #x);}
#define RUN_EXPRESSION(x) ANTH_LOGI("Running:", #x);x;
#define EVAL_EXPRESSION(x) ANTH_LOGI("Value of ",#x," is ",(x));
#define PRINT(...) ANTH_LOGI(__VA_ARGS__);

namespace Anthem::AcStage::TemplateIterators {
    using namespace Anthem::Core;
    using namespace Anthem::Core::Math;
    using namespace Anthem::Core::Math::Abbr;
    void run() {

        BEGIN_EXAMPLE("Range Zip");
            std::vector<int> wx = { 1,2,3 };
            std::set<int> st = { 8,5,1 };
            AnthemHaltonSequence bx(2, 3);
            for (const auto& [a, b, c] : AT_ZIP(wx, bx, st)) {
                PRINT(a, b, c);
            }
        END_EXAMPLE();

    }
}

int main() {
    Anthem::AcStage::TemplateIterators::run();
}