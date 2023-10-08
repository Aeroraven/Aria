cmake_minimum_required(VERSION 3.10)
project(Anthem)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCE_0 "include/*.h")
file(GLOB_RECURSE SOURCE_1 "src/*.cpp")
add_executable (Anthem "main.cpp" ${SOURCE_0} ${SOURCE_1})

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)
