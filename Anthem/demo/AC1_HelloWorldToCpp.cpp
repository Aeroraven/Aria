#include "../include/core/base/AnthemBaseImports.h"
using namespace Anthem::Core;

class Cat{
public:
    std::string name;
    Cat(std::string&& cattoName){
        this->name = cattoName;
        ANTH_LOGI("[Cat] Constructor Fired (",this->name,") @ ",(long long)(this));
    }
    Cat(const Cat& otherCat){
        this->name = otherCat.name;
        ANTH_LOGI("[Cat] Copy Constructor Fired (",this->name,") @ ",(long long)(this));
    }
    Cat(Cat&& otherCat){
        this->name = otherCat.name;
        ANTH_LOGI("[Cat] Move Constructor Fired (",this->name,") @ ",(long long)(this));
    }
    Cat& operator=(const Cat& otherCat){
        this->name = otherCat.name;
        ANTH_LOGI("[Cat] Assign Operator Fired (",this->name,") @ ",(long long)(this));
        return *this;
    }

    ~Cat(){
        ANTH_LOGI("[Cat] Destructor Fired (",this->name,") @ ",(long long)(this));
    }
    void print(){
        ANTH_LOGI(name);
    }
};

Cat getTabbyRVO(){
    Cat tabby("Tabby");
    ANTH_LOGI("Function returns");
    return tabby;
}

Cat getTabbyMove(){
    Cat tabby("Tabby");
    ANTH_LOGI("Function returns");
    return std::move(tabby);
}

Cat getTabbyStatic(){
    static Cat tabby("Tabby");
    ANTH_LOGI("Function returns");
    return tabby;
}

void askCatName1(Cat&& cat){
    cat.name = "Tiger";
    cat.print();
}

void feedCat(int& p){
    ANTH_LOGI("Feed Cat LValue",p);
}

void feedCat(const int& p){
    ANTH_LOGI("Feed Cat LValue Const",p);
}

void feedCat(int&& p){
    ANTH_LOGI("Feed Cat RValue",p);
}

void feedCat(const int&& p){
    ANTH_LOGI("Feed Cat RValue Const",p);
}

void petCat(const int& p){
    ANTH_LOGI("Pet Cat");
}

class Shape{
public:
    Shape& operator=(Shape&& rv){
        ANTH_LOGI("[Shape] Move Assign Fired @ ",(long long)(this));
        return *this;
    }
    ~Shape(){
        ANTH_LOGI("[Shape] Destructor Fired @ ",(long long)(this));
    }
};

class Color{
public:
    Color& operator=(Color&& rv){
        ANTH_LOGI("[Color] Move Assign Fired @ ",(long long)(this));
        return *this;
    }
    ~Color(){
        ANTH_LOGI("[Color] Destructor Fired @ ",(long long)(this));
    }
};

class Name{
public:
    Name& operator=(Name&& rv){
        ANTH_LOGI("[Name] Move Assign Fired @ ",(long long)(this));
        return *this;
    }
};

class Rectangle: virtual Shape, virtual Color{
public:
    Name name;
    Rectangle& operator=(Rectangle&& rv) = default;
};

class ClassM{
public:
    int C=4;
    void virtual printC(){
        ANTH_LOGI("ClassM C=",C);
    }
};
class ClassA: virtual public ClassM{
public:
    int A=2;
    void virtual print(){
        ANTH_LOGI("ClassA A=",A);
    }
};
class ClassB: virtual public ClassM{
public:
    int A=3;
    void virtual print(){
        ANTH_LOGI("ClassB A=",A);
    }
};
class ClassAB: virtual public ClassA, virtual public ClassB{
    void test(){
        ClassA& p = *this;
        ClassB& q = *this;
        p.print();
        q.print();
    }
};

class Country{

};

void meowNya(int& a){
    a+=1;
}

int main(){
    ANTH_LOGI("1=============");
    {
        Cat catA("Tabby");
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("2=============");
    {
        auto catB = Cat("Tiger");
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("3=============");
    {
        Cat catA("Tabby");
        Cat catB = catA;
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("4=============");
    {
        Cat catA("Tabby");
        Cat catB = std::move(catA);
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("5=============");
    {
        Cat catA = getTabbyRVO();
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("6=============");
    {
        Cat catA = getTabbyMove();
        ANTH_LOGI("Life cycle ends");
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("7=============");
    {
        Cat catA = getTabbyStatic();
        ANTH_LOGI("Life cycle ends");
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("8=============");
    {
        Cat catA = Cat("Garfield");
        askCatName1(std::move(catA));
        ANTH_LOGI("Name of catA:",catA.name);
        askCatName1(Cat("Tabby"));
        ANTH_LOGI("Life cycle ends");
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("9=============");
    {
        Cat&& catA = Cat("Garfield");
        Cat catB = catA;
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("10=============");
    {
        Cat&& catA = Cat("Garfield");
        {
            ANTH_LOGI("Inside scope");
            Cat catB = Cat("Tabby");
            Cat catC = Cat("Tiger");
            catA = catB;
            ANTH_LOGI("Scope ends");
        }
        ANTH_LOGI("Outer ends");
    }
    ANTH_LOGI("=============");


    ANTH_LOGI("11=============");
    {
        int a = 2;
        int&& b = 3;
        feedCat(1);
        feedCat(a);
        feedCat(b);

        feedCat(std::forward<int>(b));
        feedCat(std::move(b));
        feedCat(std::forward<int>(a));
        feedCat(std::move(a));

        feedCat(std::forward<int&>(a));
        feedCat(std::forward<int&>(b));
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("12=============");
    {
        using ilref = int&;
        using irref = int&&;

        int _a = 1;
        int _b = 2;
        int _c = 3;
        int _d = 4;

        ilref& a = _a;
        ilref&& b = _b;
        irref& c = _c;
        irref&& d = std::forward<int>(_d);

        feedCat(a);
        feedCat(b);
        feedCat(c);
        feedCat(d);
    }
    ANTH_LOGI("=============");

    
    ANTH_LOGI("13=============");
    {
        const int a = 1;
        const int& b = 2;
        const int& c = a;
        const int&& d = 4;
        const int&& e = std::forward<const int>(a);

        constexpr int f = 6;

        feedCat(a);
        feedCat(b);
        feedCat(c);
        feedCat(d);
        feedCat(e);
        feedCat(f);

        feedCat(std::forward<const int>(a));
        feedCat(std::forward<const int&>(a));
        feedCat(std::forward<const int&&>(a));

        int g = 7;
        constexpr int h = 8;
        feedCat(g);
        feedCat(static_cast<const int>(g));
        //feedCat(reinterpret_cast<const int>(g)); illegal
        feedCat(const_cast<const int&>(g));
        feedCat(const_cast<const int&&>(g));
        feedCat(std::move(const_cast<const int&>(g)));
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("14=============");
    {
        int* a0 = new int(1);  
        int* a1 = new int(2); 
        int*& b = a0;
        int*&& c = new int(3);
        ANTH_LOGI("*b=",*b, " *c=",*c);
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("15=============");
    {
        int a = 1;
        const int a2 = 0;
        auto&& b = 2;
        auto&& c = a;
        const auto&& d = 2;

        feedCat(a);
        feedCat(b);
        feedCat(c);
        feedCat(d);

        feedCat(std::forward<decltype(a)>(a));
        feedCat(std::forward<decltype(b)>(b));
        feedCat(std::forward<decltype(c)>(c));
        feedCat(std::forward<decltype(d)>(d));
    }

    ANTH_LOGI("16=============");
    {
        auto feedCatto = [&]<typename T>(T&& x){
            feedCat(x);
            feedCat(std::forward<T>(x));
            feedCat(std::forward<T&>(x));
            feedCat(std::forward<T&&>(x));
        };

        int a = 0;
        int b0 = 1;
        int& b = b0;
        int&& c = 2;

        feedCatto(a);
        feedCatto(b);
        feedCatto(c);
        feedCatto(3);
        feedCatto(std::move(a));
    }
    ANTH_LOGI("=============");
    ANTH_LOGI("17=============");
    {
        Rectangle rect;
        rect = std::move(Rectangle());
        ANTH_LOGI("Scope ends");
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("18=============");
    {
        static constexpr int a00 = 2;
        static int a01 = 4;
        const int a = 23;
        const int a2 = 46;
        int a3 = 94;

        const int* b = &a;
        b = &a2;

        int* c = &a3;
        int* const d = c;
        (*d)++;

        constexpr int m = 2;
        constexpr const int* e = &a00;
        constexpr int* g = &a01;
        auto f = e;
        auto h = &a01;
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("19=============");
    {
        auto meow = [](int& a){
            a+=1;
        };
        int a = 0;
        meow(a);

        int b = 0;
        std::function<void(int&)> meowBind;
        meowBind = std::bind(meow,std::placeholders::_1);
        meowBind(b);

        int c = 0;
        std::function<void()> meowBind2;
        meowBind2 = std::bind(meow,c);
        meowBind2();

        int d = 0;
        std::function<void()> meowBind3;
        meowBind3 = std::bind(meow,std::ref(d));
        meowBind3();


        ANTH_LOGI("A=",a);
        ANTH_LOGI("B=",b);
        ANTH_LOGI("C=",c);
        ANTH_LOGI("D=",d);
    }
    ANTH_LOGI("=============");

    ANTH_LOGI("20=============");
    {
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_real_distribution<float> unf(0.0f,1.0f);
        for(int i=0;i<10;i++){
            ANTH_LOGI(unf(generator));
        }

    }
    ANTH_LOGI("=============");
    return 0;
}