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

namespace Anthem::AcStage::Usability{

    void nullptrCallTest(int x){
        PRINT("Calls function which takes INT as input");
    }
    void nullptrCallTest(char* x){
        PRINT("Calls function which takes CHAR_PTR as input");
    }
    void nullptrCallTest2(void* x){
        PRINT("The value of ptr x is:",(unsigned long long)x);
    }
    constexpr int sumFrom1ToN(const int x){
        return (!x)?0:x+sumFrom1ToN(x-1);
    }
    const int sumFrom1ToN_2(const int x){
        return (!x)?0:x+sumFrom1ToN_2(x-1);
    }
    int sumFrom1ToN_3(int x){
        return (!x)?0:x+sumFrom1ToN_3(x-1);
    }

    class Catto{
    public:
        int a;
        int b;
        Catto(int s,int t):a(s),b(t){
            PRINT("Catto -> Constructor (int,int) is fired.");
        }
        Catto(std::initializer_list<int> x){
            PRINT("Catto -> Constructor (init_list) is fired.");
        }
    };

    class Katze{
    public:
        explicit Katze(std::initializer_list<int> x){
            PRINT("Katze -> Constructor (init_list) is fired.");
        }
        Katze(int a,int b){
            PRINT("Katze -> Constructor (int,int) is fired.");
        }
    };

    class Cattus{
    public:
        Cattus(int a,int b){
            PRINT("Cattus -> Constructor (int,int) is fired.");
        }
    };

    class Neko{
    public:
        Neko(std::initializer_list<int> x){
            PRINT("Neko -> Constructor (init_list) is fired.");
        }
        explicit Neko(int a,int b){
            PRINT("Neko -> Constructor (int,int) is fired.");
        }
    };

    double calcSum(auto x,auto y){
        auto z = x+y;
        JUDGE_EXPRESSION((std::is_same_v<decltype(x),decltype(z)>));
        JUDGE_EXPRESSION((std::is_same_v<decltype(y),decltype(z)>));
        return z;
    }

    auto calcMul(auto x,auto y) -> decltype(x*y){
        auto z = x*y;
        JUDGE_EXPRESSION((std::is_same_v<decltype(x),decltype(z)>));
        JUDGE_EXPRESSION((std::is_same_v<decltype(y),decltype(z)>));
        return z;
    }

    constexpr int sumFrom1To100(){
        int r = 0;
        for(int i=0;i<=100;i++){
            r+=i;
        }
        return r;
    }

    template<int V>
    struct FoxAge{
        constexpr static int age = V;
    };

    constexpr auto getLuckyNumber() -> double{
        return 7;
    }

    constexpr auto getLuckyNumber2() -> int{
        return 13;
    }

    constexpr decltype(auto) proxyGetLuckyNumber(){
        return getLuckyNumber();
    }

    template<typename T,typename U>
    auto getLargerCat(T x,T y){
        return x>y?x:y;
    }

    auto isLargerCat(auto x,auto y) -> bool{
        return x>y;
    }

    const bool catFinder(const int& numbers,const int groupA[],const int groupB[],bool (*comp)(const int,const int)){
        int groupAScore = 0;
        int groupBScore = 0;
        for(auto i=0;i<numbers;i++){
            if(comp(groupA[i],groupB[i])){
                groupAScore+=1;
            }else{
                groupBScore+=1;
            }
        }
        return groupAScore>groupBScore;
    }
    const int* catFinderWithRtn(const int& numbers,const int groupA[],const int groupB[],
        const bool (*finder)(const int&,const int[],const int[],bool (*)(const int,const int)),
        bool (*comp)(const int,const int)){
        auto cmpRes = finder(numbers,groupA,groupB,comp);
        if(cmpRes){
            return groupA;
        }else{
            return groupB;
        }
    }

    void demoEntry(){

        BEGIN_EXAMPLE("nullptr")
            RUN_EXPRESSION(nullptrCallTest(nullptr));
            //RUN_EXPRESSION(nullptrCallTest(NULL)); // Warning
            RUN_EXPRESSION(nullptrCallTest(0));
            RUN_EXPRESSION(nullptrCallTest2(nullptr));
            
            JUDGE_EXPRESSION(nullptr==0);
            //JUDGE_EXPRESSION(nullptr==0.0f); // Illegal
            //JUDGE_EXPRESSION(nullptr==0.0); // Illegal
            //JUDGE_EXPRESSION(nullptr==false); // Illegal
            //JUDGE_EXPRESSION(nullptr=='\0'); // Illegal
            JUDGE_EXPRESSION(nullptr==0ll);
            JUDGE_EXPRESSION(nullptr==0ull);
            JUDGE_EXPRESSION(nullptr==(char*)NULL);
            JUDGE_EXPRESSION((std::is_same<decltype(nullptr),decltype(0)>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(nullptr),char*>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(nullptr),decltype(0.0f)>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(nullptr),unsigned long long>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(nullptr),nullptr_t>::value));
        END_EXAMPLE()

        BEGIN_EXAMPLE("constexpr 1");
            int a[sumFrom1ToN(5)];
            EVAL_EXPRESSION(sizeof(a));
            int b[sumFrom1ToN_2(6)];
            EVAL_EXPRESSION(sizeof(b));
            int c[sumFrom1ToN_3(7)];
            EVAL_EXPRESSION(sizeof(c));

            int d = 5050;
            switch (d){
            case sumFrom1To100():
                PRINT("1+...+100=",sumFrom1To100());
                break;
            default:
                PRINT("1+...+100!=",sumFrom1To100());
                break;
            }

            PRINT("Fox's age is:",FoxAge<sumFrom1To100()>::age);
            
        END_EXAMPLE();

        BEGIN_EXAMPLE("If Statement");
            if constexpr (constexpr int x=sumFrom1ToN(100);x==5050){
                PRINT("Sum from 1 to 100 equals to: ",5050);
            }else{
                PRINT("Sum from 1 to 100 is not:",5050);
            }
            int a=1;
            if (a++;a==2){
                PRINT("A==2");
            }else{
                PRINT("A!=2");
            }
        END_EXAMPLE();

        BEGIN_EXAMPLE("Switch Statement");
            constexpr int a = 2;
            const int b = 14;
            constexpr int c = a+20;
            const int d = a+([](){return 100;})();
            switch (auto x=a+1;x){
            case sumFrom1ToN(2):
                PRINT("X==1+2");
                break;
            case b:
                PRINT("X==14");
                break;
            case c:
                PRINT("X==A+20");
                break;
            case d:
                PRINT("X==A+100");
                break;
            default:
                break;
            }
        END_EXAMPLE();

        BEGIN_EXAMPLE("Initializer List");
            Catto a({1,2});
            Catto b(1,2);
            Catto c{1,2};
            Catto d{{1,2}};
            Catto e = {1,2};
            Catto f = {1,2,3,4};

            Katze g({1,2});
            //Katze h = {1,2};  // Illegal
            Katze m{1,2};

            Cattus n{1,2};
            Cattus p = {1,2};
            Cattus q(1,2);

            Neko r(1,2);
            Neko s{1,2};
            Neko t = {1,2};
            Neko u({1,2});
            Neko v = {1,2,3};
        END_EXAMPLE();

        BEGIN_EXAMPLE("Structured Binding");
            auto w = std::make_tuple(114514,"Goodbye World",4000.0f,nullptr);
            auto& [a,b,c,d] = w;
            auto [e,f,g,h] = w;
            a = 1;
            g = 0.0f;
            PRINT("First Element (lref):",std::get<0>(w));
            PRINT("Third Element (rref):",std::get<2>(w));

            int z[] = {1,2,3,4};
            auto& [p,q,r,s] = z;
            PRINT("P,Q,R,S=",p,",",q,",",r,",",s);
            p = 64;
            PRINT("P=",p," Z[0]=",z[0]);
            z[0] = 92;
            PRINT("P=",p," Z[0]=",z[0]);

            JUDGE_EXPRESSION((std::is_same<decltype(p),int>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(p),int&>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(p),int&&>::value));

            JUDGE_EXPRESSION((std::is_same<int&,int&&>::value));
            JUDGE_EXPRESSION((std::is_same<int&,int>::value));
            JUDGE_EXPRESSION((std::is_same<int,int&&>::value));
            
            struct {
                int cat=1, dog=2;
                float cactus=3.0f;
                int lynx[2] = {1,2};
            } bio;
            auto& [a1,a2,a3,a4] = bio; 
            PRINT("Cactus=",a3);

        END_EXAMPLE();

        BEGIN_EXAMPLE("Left Value Reference");
            int&& a = 12;
            int& b = a;

            JUDGE_EXPRESSION((std::is_same<decltype(a),int>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(a),int&>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(a),int&&>::value));
            
            JUDGE_EXPRESSION((std::is_same<decltype(b),int>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(b),int&>::value));
            JUDGE_EXPRESSION((std::is_same<decltype(b),int&&>::value));
        END_EXAMPLE();

        BEGIN_EXAMPLE("std::tie");
            auto z = std::make_tuple(1,2,3);
            int a,b,c;
            std::tie(a,b,c) = z;
            a = 4;
            PRINT("A=",a,"Z[0]=",std::get<0>(z));

            auto y = std::make_tuple(1,2,3);
            auto [d,e,f] = y;
            d = 9;
            PRINT("D=",d,"Y[0]=",std::get<0>(y));

            auto x = std::make_tuple(1,2,3);
            auto& [p,q,r] = x;
            p = 9;
            PRINT("P=",p,"X[0]=",std::get<0>(x));

            std::tie(p,q,r) = y;
            PRINT("P=",p," X[0]=",std::get<0>(x)," Y[0]=",std::get<0>(y));
            p = 12;
            PRINT("P=",p," X[0]=",std::get<0>(x)," Y[0]=",std::get<0>(y));

        END_EXAMPLE();

        BEGIN_EXAMPLE("Type Inference I: auto & decltype");
            RUN_EXPRESSION(calcSum(1.0f,2.0));
            RUN_EXPRESSION(calcSum(1,2ll));
            RUN_EXPRESSION(calcSum(1u,2));
            RUN_EXPRESSION(calcSum(2u,45ll));
            RUN_EXPRESSION(calcSum(114ull,514.0));
        END_EXAMPLE();

        BEGIN_EXAMPLE("Type Inference II: Trailing");
            auto x = calcMul(114u,514ll);
            auto y = calcMul(114u,short(514));
            JUDGE_EXPRESSION((std::is_same_v<decltype(x),long long>));
            JUDGE_EXPRESSION((std::is_same_v<decltype(y),unsigned>));
        END_EXAMPLE();

        BEGIN_EXAMPLE("Type Inference III: decltype(auto)");
            JUDGE_EXPRESSION((std::is_same_v<decltype(proxyGetLuckyNumber()),double>));
            JUDGE_EXPRESSION((std::is_same_v<decltype(proxyGetLuckyNumber()),double&&>));
        END_EXAMPLE();

        BEGIN_EXAMPLE("Range-based Loop");
            std::array<int,3> a = {1,2,3};
            for(const auto& x:a){
                PRINT(x);
            }

            int b[] = {4,5,6};
            for(const auto& x:b){
                PRINT(x);
            }

            int c[] = {6,7,8};
            for(auto incr = 1;auto& x:c){
                x+=incr;
            }
            for(const auto& x:c){
                PRINT(x);
            }
        END_EXAMPLE();

        BEGIN_EXAMPLE("Type Alias & Alias Template I: typedef 1");
            typedef struct {
                int real = 4;
                int imag = 2;
            } CatComplex, *CatComplexPtr, &CatComplexLRef, &&CatComplexRRef;
            CatComplexPtr a = new CatComplex();
            CatComplexLRef b = *a;
            PRINT("Imag,Real=",a->imag,",",a->real);

            typedef int (*MaxCatSelector)(int,int);
            MaxCatSelector maxCatSelector = getLargerCat<int,int>;
            PRINT("Larger Cat is:",maxCatSelector(114,514));

            typedef bool (*CatComparer)(const int, const int);
            typedef const int* (*CatFinderWithReturnValue)(const int&,const int[],const int[],
                const bool (*)(const int&,const int[],const int[],CatComparer), CatComparer);

            CatFinderWithReturnValue myFinder = catFinderWithRtn;
            CatComparer myComparer = isLargerCat;
            int redCats[] = {1,2,3,4};
            int blueCats[] = {5,6,7,1};
            auto bestCats = myFinder(4,redCats,blueCats,catFinder,myComparer);
            for(int i=0;i<4;i++){
                PRINT("Best cat is",i[bestCats]);
            }
        END_EXAMPLE();

        BEGIN_EXAMPLE("Type Alias & Alias Template II: typedef 2");
            int p = 19;
            int q = 25;
            
            const int* pp = &p;
            PRINT("PP->",*pp);
            pp = &q;
            PRINT("PP->",*pp);

            typedef int* IntPtr;

            const IntPtr pp2 = &p;
            PRINT("PP2->",*pp2);
            *pp2 = 114514;
            PRINT("PP2->",*pp2);

            JUDGE_EXPRESSION((std::is_same_v<decltype(pp2),const int*>));
            JUDGE_EXPRESSION((std::is_same_v<decltype(pp2),int* const>));
            JUDGE_EXPRESSION((std::is_same_v<decltype(pp2),const int* const>));
        END_EXAMPLE();

        BEGIN_EXAMPLE("Type Alias & Alias Template III: using");
            using IntPtr = int*;
            int p = 19;
            int q = 25;
            const IntPtr pp2 = &q;
            
            JUDGE_EXPRESSION((std::is_same_v<decltype(pp2),const int*>));
            JUDGE_EXPRESSION((std::is_same_v<decltype(pp2),int* const>));
            JUDGE_EXPRESSION((std::is_same_v<decltype(pp2),const int* const>));
        END_EXAMPLE();

    }
}


int main(){
    Anthem::AcStage::Usability::demoEntry();
    return 0;
}