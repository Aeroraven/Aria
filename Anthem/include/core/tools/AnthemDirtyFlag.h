#pragma once
#include "../base/AnthemBaseImports.h"

namespace Anthem::Core::Tool{
    template<typename T>
    class AnthemDirtyFlag{
    private:
        T data;
        bool dirty;
        std::optional<std::function<void(T&)>> updater;
    public:
        T get_(){
            if(dirty){
                if(updater.has_value()){
                    updater.value()(this->data);
                }else{
                    ANTH_LOGE("");
                }
                dirty = false;
            }
            return data;
        }
        const T& get(){
            if(dirty){
                if(updater.has_value()){
                    updater.value()(this->data);
                }else{
                    ANTH_LOGE("");
                }
                dirty = false;
            }
            return data;
        }
        AnthemDirtyFlag<T>& operator=(const T& other){
            data = other;
            dirty = false;
            return *this;
        }
        bool setUpdater(std::function<void(T&)> updater){
            this->updater = updater;
            return true;
        }
        bool markDirty(){
            this->dirty = true;
            return true;
        }
    };
}