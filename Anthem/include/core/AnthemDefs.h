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

#define ANTH_SHARED_PTR(T) std::shared_ptr<T>
#define ANTH_UNIQUE_PTR(T) std::unique_ptr<T>
#define ANTH_MAKE_SHARED(T) std::make_shared<T>
#define ANTH_MAKE_UNIQUE(T) std::make_unique<T>
#define ANTH_UNSAFE_PTR(T) T*
#define ANTH_MAKE_UNSAFE(T) new T

#define ANTH_LOGI Anthem::Core::AnthemLogger::getInstance().logi
#define ANTH_ASSERT Anthem::Core::AnthemLogger::getInstance().assert
#define ANTH_ASSERT_FALLBACK Anthem::Core::AnthemLogger::getInstance().assert_fallback
#define ANTH_LOGI_IF Anthem::Core::AnthemLogger::getInstance().logiif
#define ANTH_LOGW Anthem::Core::AnthemLogger::getInstance().logw

#define ANTH_LOGE Anthem::Core::AnthemLogger::getInstance().loge
#define ANTH_DEPRECATED_MSG Anthem::Core::AnthemLogger::getInstance().loge("This function is deprecated")