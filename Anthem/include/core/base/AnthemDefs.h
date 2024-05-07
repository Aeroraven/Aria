#pragma once
#include <memory>
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
#include <random>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <variant>
#include <unordered_set>
#include <chrono>
#include <ranges>
#include <typeinfo>
#include <sstream>
#include <deque>

// Flags
//#define AT_FEATURE_RAYTRACING_ENABLED 1
//#define AT_FEATURE_CUDA_ENABLED 1

// Core
#ifndef AT_FEATURE_CUDA_ENABLED
    #ifdef _HAS_CXX23
	    #define AT_CXX23_ENABLED 1
    #endif
    #ifndef _HAS_CXX23
        #if __cplusplus >= 202302L
            #define AT_CXX23_ENABLED 1
        #endif
    #endif
#endif

// Intellisense for cuda
#ifdef __INTELLISENSE__
    #define AT_KARG2(grid, block)
    #define AT_KARG3(grid, block, sh_mem)
    #define AT_KARG4(grid, block, sh_mem, stream)
    #define __CUDACC__
#else
    #ifdef __CUDACC__
	    #define AT_KARG2(grid, block) <<<grid, block>>>
	    #define AT_KARG3(grid, block, sh_mem) <<<grid, block, sh_mem>>>
	    #define AT_KARG4(grid, block, sh_mem, stream) <<<grid, block, sh_mem, stream>>>
    #else
        #define AT_KARG2(grid, block)
        #define AT_KARG3(grid, block, sh_mem)
        #define AT_KARG4(grid, block, sh_mem, stream)
    #endif
#endif

#ifdef AT_CXX23_ENABLED
#include <stacktrace>
#endif

// Options
#define AT_ENABLE_LOG 1 
#define AT_LOG_IGNORE_VERBOSE 1

// Backend
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


// External Modules
#include <ft2build.h>
#include FT_FREETYPE_H
#define AT_EXT_FREETYPE_ENABLED

#include <opencv2/core/core.hpp>          
#include <opencv2/imgcodecs/imgcodecs.hpp>    
#include <opencv2/imgproc.hpp>
#define AT_EXT_OPENCV_ENABLED

// WorkDir
#define ANTH_SHADER_DIR "C:\\WR\\Aria\\Anthem\\shader\\glsl\\"
#define ANTH_SHADER_DIR_HLSL "C:\\WR\\Aria\\Anthem\\shader\\hlsl\\"
#define ANTH_ASSET_DIR "C:\\WR\\Aria\\Anthem\\assets\\"
#define ANTH_ASSET_TEMP_DIR "C:\\WR\\Aria\\Anthem\\assets\\"

// Pointerrs
#define ANTH_SHARED_PTR(T) std::shared_ptr<T>
#define ANTH_UNIQUE_PTR(T) std::unique_ptr<T>
#define ANTH_MAKE_SHARED(T) std::make_shared<T>
#define ANTH_MAKE_UNIQUE(T) std::make_unique<T>
#define ANTH_UNSAFE_PTR(T) T*
#define ANTH_MAKE_UNSAFE(T) new T

// Compiler Config
#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

// Math Constants
#define AT_PI 3.1415926535897934384626

// Base Utilities

namespace Anthem::Core::BaseUtility {
    template<typename... Args>
    class AtZipContainer {
    private:
        template <typename T>
        using ContainerIteratorTp = decltype(std::begin(std::declval<T&>()));
        template <typename T>
        using ContainerIteratorValTp = std::iterator_traits<ContainerIteratorTp<T>>::value_type;

        using GroupIteratorTp = std::tuple<ContainerIteratorTp<Args>...>;
        using GroupIteratorValTp = std::tuple<ContainerIteratorValTp<Args>...>;
        
        std::unique_ptr<std::tuple<Args...>> tuples;
    public:
        class iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = GroupIteratorValTp;
            using reference = const GroupIteratorValTp&;
            using pointer = GroupIteratorValTp*;

        private:
            std::unique_ptr<GroupIteratorTp> curIters;
        
        public:
            iterator(const GroupIteratorTp& iterators) {
                this->curIters = std::make_unique<GroupIteratorTp>(iterators);
            }
            iterator (const iterator& other) {
                GroupIteratorTp pIters = *other.curIters;
                this->curIters = std::make_unique<GroupIteratorTp>(pIters);
            }
            iterator& operator++() {
                std::apply([](auto&... p) {(++p, ... ); }, *(this->curIters));
                return *this;
            }
            iterator operator++(int) {
                iterator copyIter(*this);
                std::apply([](auto&... p) {(++p, ...); }, *(this->curIters));
                return copyIter;
            }
            bool operator==(iterator p) const {
                return *curIters == *p.curIters;
            }
            bool operator!=(iterator p) const {
                return *curIters != *p.curIters;
            }
            GroupIteratorValTp operator*() const {
                return std::apply([](auto&... p) {return std::make_tuple(*p...); }, *(this->curIters));
            }
        };
    public:
        AtZipContainer(Args... args) {
            this->tuples = std::make_unique<std::tuple<Args...>>(std::make_tuple(args...));
        }
        iterator begin() {
            return iterator(std::apply([](auto&... p) { return std::make_tuple((p.begin())...); }, *(this->tuples)));
        }
        iterator end() {
            return iterator(std::apply([](auto&... p) { return std::make_tuple((p.end())...); }, *(this->tuples)));
        }

    };
}
#define AT_ZIP(...)  Anthem::Core::BaseUtility::AtZipContainer(__VA_ARGS__)
#define AT_RANGE(s,t) (std::views::iota(s,t))
#define AT_RANGE2(t) (std::views::iota(static_cast<std::remove_reference_t<decltype((t))>>(0),(t)))
#define AT_CLAMP(v,s,t) ((v)>(s)?( (v)<(t)?(v):(t) ):(s));

#define AT_ALIGN(s,t) (((s)+(t)-1)&~((t)-1))
#define AT_LOWBIT(s) ((s)&~(s))

#define ANTH_CLASSTP std::remove_reference<decltype(*this)>::type

#define AT_CHECKRES(expr) if(auto atRes = (expr);atRes!=VK_SUCCESS){ANTH_LOGE("Returned:",atRes);}

#ifdef AT_CXX23_ENABLED
    #define ANTH_CLASSNAME (Anthem::Core::AnthemLogger::getInstance().classNameTrack(std::stacktrace::current().at(0).description()).c_str())
#else
    #ifdef _MSC_VER
        #define ANTH_CLASSNAME (Anthem::Core::AnthemLogger::getInstance().className(__FUNCSIG__).c_str())
    #else
        #define ANTH_CLASSNAME (Anthem::Core::AnthemLogger::getInstance().className(__PRETTY_FUNCTION__).c_str())
    #endif
#endif


//===================== END OF CORE DEFINITIONS ===================== 

// Logger
#ifdef AT_ENABLE_LOG
    #ifdef AT_LOG_IGNORE_VERBOSE
        #define ANTH_LOGV(...)
    #else
        #define ANTH_LOGV(...) Anthem::Core::AnthemLogger::getInstance().logv2(ANTH_CLASSNAME,std::make_tuple(__VA_ARGS__))
    #endif
    #define ANTH_LOGI(...) Anthem::Core::AnthemLogger::getInstance().logi2(ANTH_CLASSNAME,std::make_tuple(__VA_ARGS__))
    #define ANTH_LOGW(...) Anthem::Core::AnthemLogger::getInstance().logw2(ANTH_CLASSNAME,std::make_tuple(__VA_ARGS__))
    #define ANTH_LOGE(...) Anthem::Core::AnthemLogger::getInstance().loge2(ANTH_CLASSNAME,std::make_tuple(__VA_ARGS__))
    #define ANTH_DEPRECATED_MSG ANTH_LOGE("This function is deprecated")
    #define ANTH_ASSERT(x,...) Anthem::Core::AnthemLogger::getInstance().log_assert2(ANTH_CLASSNAME,x,std::make_tuple(__VA_ARGS__))
    #define ANTH_ASSERT_FALLBACK Anthem::Core::AnthemLogger::getInstance().assert_fallback
    #define ANTH_CHECK_NULL(x) ANTH_ASSERT((x)!=nullptr, #x ," is nullpointer")
    #define ANTH_LOGI_IF Anthem::Core::AnthemLogger::getInstance().logiif
#else
    #define ANTH_LOGV(...) 
    #define ANTH_LOGI(...) 
    #define ANTH_LOGW(...)
    #define ANTH_LOGE(...)
    #define ANTH_DEPRECATED_MSG
    #define ANTH_ASSERT(x,...) Anthem::Core::AnthemLogger::getInstance().log_assert2(ANTH_CLASSNAME,x,std::make_tuple(__VA_ARGS__))
    #define ANTH_ASSERT_FALLBACK Anthem::Core::AnthemLogger::getInstance().assert_fallback
    #define ANTH_CHECK_NULL(x)
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

    template<class SrcTp, int SrcDm, int SrcSz>
    concept ATpIsdUniMatVeci = ATpIsdUniEquals<SrcTp, SrcDm, SrcSz, int, 1, 2, 1, 4>;


    template<class SrcTp,int SrcDm,int SrcSz>
    concept ATpIsdValidUniform = ATpIsdUniMatVecf<SrcTp,SrcDm,SrcSz>;

    template<class SrcTp,int SrcDm,int SrcSz,int SrcArrSz>
    concept ATpIsdValidUniformWithArr = (ATpIsdUniMatVecf<SrcTp,SrcDm,SrcSz>|| ATpIsdUniMatVeci<SrcTp, SrcDm, SrcSz>) && (SrcArrSz>0);
}


