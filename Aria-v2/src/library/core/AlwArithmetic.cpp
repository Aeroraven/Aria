#include "AlwCoreDefs.h"

#define ALW_VECTOR_TYPE float

class AlwVector{
private:
    std::unique_ptr<ALW_VECTOR_TYPE> _value;
    int _dimen;
public:
    AlwVector(int dimen){
        _dimen = dimen;
        _value = std::make_unique<ALW_VECTOR_TYPE>(dimen);
    }
    AlwVector(ALW_VECTOR_TYPE* data, int dimen){
        _dimen = dimen;
        _value = std::make_unique<ALW_VECTOR_TYPE>(dimen);
        for(int i = 0; i < dimen; i++){
            _value.get()[i] = data[i];
        }
    }
    AlwVector(const AlwVector& other){
        _dimen = other._dimen;
        _value = std::make_unique<ALW_VECTOR_TYPE>(_dimen);
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] = other._value.get()[i];
        }
    }

#ifdef ALW_ENABLE_WASM

    AlwVector(emscripten::val data){
        _dimen = data["length"].as<int>();
        _value = std::make_unique<ALW_VECTOR_TYPE>(_dimen);
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] = data[i].as<ALW_VECTOR_TYPE>();
        }
    }

#endif

    auto at(int index) const{
        return _value.get()[index];
    }
    auto setVal(int index, ALW_VECTOR_TYPE val){
        _value.get()[index] = val;
    }
    auto add(const AlwVector& other) const{
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] += other._value.get()[i];
        }
    }
    auto add_(const AlwVector& other){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] + other._value.get()[i]);
        }
        return result;
    }
    auto addScalar(ALW_VECTOR_TYPE scalar) const{
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] += scalar;
        }
    }
    auto addScalar_(ALW_VECTOR_TYPE scalar){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] + scalar);
        }
        return result;
    }
    auto addScaled_(const AlwVector& other, ALW_VECTOR_TYPE scalar){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] + other._value.get()[i] * scalar);
        }
        return result;
    }
    auto sub(const AlwVector& other) const{
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] -= other._value.get()[i];
        }
    }
    auto sub_(const AlwVector& other){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] - other._value.get()[i]);
        }
        return result;
    }
    auto subScalar(ALW_VECTOR_TYPE scalar) const{
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] -= scalar;
        }
    }
    auto subScalar_(ALW_VECTOR_TYPE scalar){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] - scalar);
        }
        return result;
    }
    auto mul(const AlwVector& other) const{
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] *= other._value.get()[i];
        }
    }
    auto mul_(const AlwVector& other){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] * other._value.get()[i]);
        }
        return result;
    }
    auto mulScalar(ALW_VECTOR_TYPE scalar) const{
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] *= scalar;
        }
    }
    auto mulScalar_(ALW_VECTOR_TYPE scalar){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] * scalar);
        }
        return result;
    }
    auto div(const AlwVector& other) const{
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] /= other._value.get()[i];
        }
    }
    auto div_(const AlwVector& other){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] / other._value.get()[i]);
        }
        return result;
    }
    auto divScalar(ALW_VECTOR_TYPE scalar) const{
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] /= scalar;
        }
    }
    auto divScalar_(ALW_VECTOR_TYPE scalar){
        auto result = std::make_shared<AlwVector>(_dimen);
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] / scalar);
        }
        return result;
    }
    auto dot(const AlwVector& other) const{
        ALW_VECTOR_TYPE result = 0;
        for(int i = 0; i < _dimen; i++){
            result += _value.get()[i] * other._value.get()[i];
        }
        return result;
    }
    auto sum() const{
        ALW_VECTOR_TYPE result = 0;
        for(int i = 0; i < _dimen; i++){
            result += _value.get()[i];
        }
        return result;
    }
    auto len() const{
        ALW_VECTOR_TYPE result = 0;
        for(int i = 0; i < _dimen; i++){
            result += _value.get()[i] * _value.get()[i];
        }
        return sqrt(result);
    }
    auto normalize() const{
        auto result = std::make_shared<AlwVector>(_dimen);
        auto len = this->len();
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] / len);
        }
        return result;
    }
    auto normalize_(){
        auto result = std::make_shared<AlwVector>(_dimen);
        auto len = this->len();
        for(int i = 0; i < _dimen; i++){
            result->setVal(i, _value.get()[i] / len);
        }
        return result;
    }
    auto fromVector(const AlwVector& other){
        _dimen = other._dimen;
        _value = std::make_unique<ALW_VECTOR_TYPE>(_dimen);
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] = other._value.get()[i];
        }
    }


#ifdef ALW_ENABLE_WASM

    auto fromArray(emscripten::val data){
        _dimen = data["length"].as<int>();
        _value = std::make_unique<ALW_VECTOR_TYPE>(_dimen);
        for(int i = 0; i < _dimen; i++){
            _value.get()[i] = data[i].as<ALW_VECTOR_TYPE>();
        }
    }

    auto toArray(){
        auto result = emscripten::val::array();
        for(int i = 0; i < _dimen; i++){
            result.set(i, _value.get()[i]);
        }
        return result;
    }

#endif

    auto dim(){
        return _dimen;
    }
    auto cross(const AlwVector& other) const{
        auto result = std::make_shared<AlwVector>(_dimen);
        result->setVal(0, _value.get()[1] * other._value.get()[2] - _value.get()[2] * other._value.get()[1]);
        result->setVal(1, _value.get()[2] * other._value.get()[0] - _value.get()[0] * other._value.get()[2]);
        result->setVal(2, _value.get()[0] * other._value.get()[1] - _value.get()[1] * other._value.get()[0]);
        return result;
    }
    auto cross_(const AlwVector& other){
        auto result = std::make_shared<AlwVector>(_dimen);
        result->setVal(0, _value.get()[1] * other._value.get()[2] - _value.get()[2] * other._value.get()[1]);
        result->setVal(1, _value.get()[2] * other._value.get()[0] - _value.get()[0] * other._value.get()[2]);
        result->setVal(2, _value.get()[0] * other._value.get()[1] - _value.get()[1] * other._value.get()[0]);
        return result;
    }
    auto static create(int dimen){
        return std::make_shared<AlwVector>(dimen);
    }

#ifdef ALW_ENABLE_WASM

    auto static createA(emscripten::val data){
        return std::make_shared<AlwVector>(data);
    }

#endif

};

#ifdef ALW_ENABLE_EMCC_BINDINGS

EMSCRIPTEN_BINDINGS(alw_vector_bind){
    class_<AlwVector>("AlwVector")
        .constructor<int>()
        .function("at", &AlwVector::at)
        .function("setVal", &AlwVector::setVal)
        .function("add", &AlwVector::add)
        .function("add_", &AlwVector::add_)
        .function("addScalar", &AlwVector::addScalar)
        .function("addScalar_", &AlwVector::addScalar_)
        .function("addScaled_", &AlwVector::addScaled_)
        .function("sub", &AlwVector::sub)
        .function("sub_", &AlwVector::sub_)
        .function("subScalar", &AlwVector::subScalar)
        .function("subScalar_", &AlwVector::subScalar_)
        .function("mul", &AlwVector::mul)
        .function("mul_", &AlwVector::mul_)
        .function("mulScalar", &AlwVector::mulScalar)
        .function("mulScalar_", &AlwVector::mulScalar_)
        .function("div", &AlwVector::div)
        .function("div_", &AlwVector::div_)
        .function("divScalar", &AlwVector::divScalar)
        .function("divScalar_", &AlwVector::divScalar_)
        .function("dot", &AlwVector::dot)
        .function("sum", &AlwVector::sum)
        .function("len", &AlwVector::len)
        .function("normalize", &AlwVector::normalize)
        .function("normalize_", &AlwVector::normalize_)
        .function("fromArray", &AlwVector::fromArray)
        .function("fromVector", &AlwVector::fromVector)
        .function("toArray", &AlwVector::toArray)
        .function("dim", &AlwVector::dim)
        .function("cross", &AlwVector::cross)
        .function("cross_", &AlwVector::cross_)
        .class_function("create", &AlwVector::create)
        .class_function("createA", &AlwVector::createA);
}

#endif