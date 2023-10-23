cmake_minimum_required(VERSION 3.10)
project(AnthemCppTestbed)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
message(${Vulkan_LIBRARY})
message(${Vulkan_INCLUDE_DIR})
find_package(Vulkan REQUIRED)

set(BASE_GLFW_PATH C:/WR/Aria/Anthem/lib/glfw3/)
set(GLFW_LIBRARIES_2 ${BASE_GLFW_PATH}/lib-mingw-w64/libglfw3.a)
set(GLFW_LIBRARIES_3 ${BASE_GLFW_PATH}/lib-mingw-w64/libglfw3dll.a)
set(GLFW_INCLUDE_DIRS C:/WR/Aria/Anthem/lib/glfw3/include)
include_directories(${GLFW_INCLUDE_DIRS})

add_executable(CppTest "main.cpp")
target_link_libraries(CppTest Vulkan::Vulkan ${GLFW_LIBRARIES_2} ${GLFW_LIBRARIES_3})