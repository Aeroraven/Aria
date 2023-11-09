#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <set>
#include <functional>
#include <fstream>
#include <ctime>
#include <regex>
#include <concepts>
#include <cmath>
#include <cstring>
#include <GLFW/glfw3.h>
#include <random>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <variant>
#include <unordered_set>
#include <chrono>

// Pointerrs
#define ANTH_SHARED_PTR(T) std::shared_ptr<T>
#define ANTH_UNIQUE_PTR(T) std::unique_ptr<T>
#define ANTH_MAKE_SHARED(T) std::make_shared<T>
#define ANTH_MAKE_UNIQUE(T) std::make_unique<T>
#define ANTH_UNSAFE_PTR(T) T*
#define ANTH_MAKE_UNSAFE(T) new T

// Math Constants
#define AT_PI 3.1415926535897934384626

// Sugars
#define ANTH_CLASSTP std::remove_reference<decltype(*this)>::type
#ifdef _MSC_VER
#define ANTH_CLASSNAME (Anthem::Core::AnthemLogger::getInstance().className(__FUNCSIG__).c_str())
#else
#define ANTH_CLASSNAME (Anthem::Core::AnthemLogger::getInstance().className(__PRETTY_FUNCTION__).c_str())
#endif

// Logger
#define ANTH_ENABLE_LOG

#ifdef ANTH_ENABLE_LOG
    #define ANTH_LOGV(...) Anthem::Core::AnthemLogger::getInstance().logv2(ANTH_CLASSNAME,std::make_tuple(__VA_ARGS__))
    #define ANTH_LOGI(...) Anthem::Core::AnthemLogger::getInstance().logi2(ANTH_CLASSNAME,std::make_tuple(__VA_ARGS__))
    #define ANTH_LOGW(...) Anthem::Core::AnthemLogger::getInstance().logw2(ANTH_CLASSNAME,std::make_tuple(__VA_ARGS__))
    #define ANTH_LOGE(...) Anthem::Core::AnthemLogger::getInstance().loge2(ANTH_CLASSNAME,std::make_tuple(__VA_ARGS__))
    #define ANTH_DEPRECATED_MSG ANTH_LOGE("This function is deprecated")
    #define ANTH_ASSERT(x,...) Anthem::Core::AnthemLogger::getInstance().log_assert2(ANTH_CLASSNAME,x,std::make_tuple(__VA_ARGS__))
    #define ANTH_ASSERT_FALLBACK Anthem::Core::AnthemLogger::getInstance().assert_fallback
    #define ANTH_LOGI_IF Anthem::Core::AnthemLogger::getInstance().logiif
#else
    #define ANTH_LOGV(...) 
    #define ANTH_LOGI(...) 
    #define ANTH_LOGW(...)
    #define ANTH_LOGE(...)
    #define ANTH_DEPRECATED_MSG
    #define ANTH_ASSERT(x,...) Anthem::Core::AnthemLogger::getInstance().log_assert2(ANTH_CLASSNAME,x,std::make_tuple(__VA_ARGS__))
    #define ANTH_ASSERT_FALLBACK Anthem::Core::AnthemLogger::getInstance().assert_fallback
    #define ANTH_LOGI_IF(...)
#endif

#define ANTH_TODO(...)

// Default
#define ANTH_ERROR_RAISE_DEFAULT_FUNC ([](){ANTH_LOGE("Unspecified error handler");exit(1);})

// Concepts
namespace Anthem::Core::TmplDefs{
    template<class T,class... U>
    concept ATpdAnyOf = (std::same_as<T,U> || ...);

    template<class T,class... U>
    concept ATpdAllAre = (std::same_as<T,U> && ...);

    template<typename T,T d,T x,T y>
    concept ATpdValueInRange = (d>=x) && (d<=y);

    //Attributes Templates
    template<class SrcTp,int SrcSz, class DstTp, int DstSz>
    concept ATpdIsdAttrEquals = (std::same_as<SrcTp,DstTp> && (SrcSz==DstSz));

    template<class SrcTp,int SrcSz, class DstTp, int DstLb,int DstUb>
    concept ATpdIsdAttrEqualsEx = (std::same_as<SrcTp,DstTp> && (SrcSz<=DstUb) && (SrcSz>=DstLb));

    template<class SrcTp,int SrcSz> 
    concept ATpIsdAttrVecf = ATpdIsdAttrEqualsEx<SrcTp,SrcSz,float,1,4>;

    template<class SrcTp,int SrcSz> 
    concept ATpIsdValidAttr = ATpIsdAttrVecf<SrcTp,SrcSz>;

    //Uniform Templates
    template<class SrcTp,int SrcDm,int SrcSz,class DstTp,int DstDmLb,int DstDmUb,int DstSzLb,int DstSzUb>
    concept ATpIsdUniEquals = (std::same_as<SrcTp,DstTp> && (SrcDm<=DstDmUb) && (SrcDm>=DstDmLb) && (SrcSz<=DstSzUb) && (SrcSz>=DstSzLb));

    template<class SrcTp,int SrcDm,int SrcSz>
    concept ATpIsdUniMatVecf = ATpIsdUniEquals<SrcTp,SrcDm,SrcSz,float,1,2,1,4>;

    template<class SrcTp,int SrcDm,int SrcSz>
    concept ATpIsdValidUniform = ATpIsdUniMatVecf<SrcTp,SrcDm,SrcSz>;

    template<class SrcTp,int SrcDm,int SrcSz,int SrcArrSz>
    concept ATpIsdValidUniformWithArr = ATpIsdUniMatVecf<SrcTp,SrcDm,SrcSz> && (SrcArrSz>0);
}
