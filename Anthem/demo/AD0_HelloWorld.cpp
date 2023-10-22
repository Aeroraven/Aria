#include "../include/core/base/AnthemBaseImports.h"
using namespace Anthem::Core;

class Cat{
public:
    std::string _name;
    Cat(std::string name){
        _name = name;
        ANTH_LOGI("Cat constructor",name, " @",(unsigned long long)this);
    }
    Cat(const Cat& other){
        this->_name = other._name;
        ANTH_LOGI("Cat copy constructor",other._name);
    }
    Cat operator=(const Cat& other){
        this->_name = other._name;
        ANTH_LOGI("Cat operator=");
        return *this;
    }
    void print(){
        ANTH_LOGI("Cat print",_name, " @",(unsigned long long)this);
    }
};

Cat& getCat(){
    static Cat cat("Garfield");
    //ANTH_LOGI("Func Returns");
    return cat;
}

Cat getCat2(){
    static Cat cat("Tabby");
    //ANTH_LOGI("Func Returns");
    return cat;
}

Cat getCat3(){
    static Cat cat("Tabby");
    return cat;
}


int main(){
    ANTH_LOGI("============");
    auto a = getCat2();
    a.print();
    ANTH_LOGI("============");
    auto b = getCat();
    b.print();
    ANTH_LOGI("============");
    auto c = Cat("Tom");
    c.print();
    ANTH_LOGI("============");
    auto d = c;
    d.print();
    ANTH_LOGI("============");
    auto e = getCat3();
    e.print();
    ANTH_LOGI("============");
    //ANTH_LOGI(a);

    return 0;
}