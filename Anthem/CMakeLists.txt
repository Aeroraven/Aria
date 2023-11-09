cmake_minimum_required(VERSION 3.10)
project(Anthem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -fmodules-ts")

set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

message(CMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
message(CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS})

set(Vulkan_LIBRARY C:/VulkanSDK/1.3.261.1/Lib/vulkan-1.lib)
set(Vulkan_INCLUDE_DIR C:/VulkanSDK/1.3.261.1/Include)
find_package(Vulkan REQUIRED)

set(glfw3_DIR "C:/Program Files (x86)/GLFW/lib/cmake/glfw3/")
set(glfw3_LIB "C:/Program Files (x86)/GLFW/lib/glfw3.lib")
find_package(glfw3 3.3 REQUIRED)

include_directories("C:/Program Files (x86)/GLFW/include")

file(GLOB_RECURSE SOURCE_0 "include/*.h")
file(GLOB_RECURSE SOURCE_1 "src/*.cpp")
file(GLOB_RECURSE SOURCE_2 "include/external_ref/*.h")

file(GLOB SOURCE_STAGES "demo/*.cpp")

foreach( SRC_FILE_NAME ${SOURCE_STAGES} )
    get_filename_component( SRC_FILE_NAME_SHORT ${SRC_FILE_NAME} NAME_WE )
    add_executable( ${SRC_FILE_NAME_SHORT} ${SRC_FILE_NAME} ${SOURCE_0} ${SOURCE_1})
    target_link_libraries( ${SRC_FILE_NAME_SHORT} Vulkan::Vulkan ${glfw3_LIB})
endforeach( SRC_FILE_NAME ${SOURCE_STAGES} )
