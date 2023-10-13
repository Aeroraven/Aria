#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <optional>
#include <set>
#include <functional>
#include <fstream>

// Pointerrs
#define ANTH_SHARED_PTR(T) std::shared_ptr<T>
#define ANTH_UNIQUE_PTR(T) std::unique_ptr<T>
#define ANTH_MAKE_SHARED(T) std::make_shared<T>
#define ANTH_MAKE_UNIQUE(T) std::make_unique<T>
#define ANTH_UNSAFE_PTR(T) T*
#define ANTH_MAKE_UNSAFE(T) new T

// Sugars
#define ANTH_CLASSTP std::remove_reference<decltype(*this)>::type
#define ANTH_CLASSNAME (Anthem::Core::AnthemLogger::getInstance().className(__PRETTY_FUNCTION__).c_str())

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

// Default
#define ANTH_ERROR_RAISE_DEFAULT_FUNC ([](){ANTH_LOGE("Unspecified error handler");exit(1);})

// String Literals
