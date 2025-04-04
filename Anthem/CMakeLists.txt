cmake_minimum_required(VERSION 3.10)
project(Anthem LANGUAGES CUDA CXX)

# Compile Arguments
#add_compile_definitions(AT_FEATURE_RAYTRACING_ENABLED)

# Configs

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(AT_REQ_VULKAN_BASE "C:/VulkanSDK/1.3.261.1")
set(AT_REQ_GLFW3_BASE "C:/Program Files (x86)/GLFW/")
set(AT_REQ_FREETYPE_BASE "C:/Program Files (x86)/freetype/")
set(AT_REQ_OPENCV_INSTALL "C:/Users/Huang/Desktop/Work/OpenCV/opencv/sources/build/install")
set(AT_REQ_CUDA_INCLUDE "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.4/include")
set(AT_REQ_CUDACRT_INCLUDE "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.4/include/crt")

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
elseif(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

message(CMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR})
message(CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS})

set(Vulkan_LIBRARY ${AT_REQ_VULKAN_BASE}/Lib/vulkan-1.lib)
set(Vulkan_INCLUDE_DIR ${AT_REQ_VULKAN_BASE}/Include)
find_package(Vulkan REQUIRED)

set(glfw3_DIR ${AT_REQ_GLFW3_BASE}/lib/cmake/glfw3/)
if(MSVC)
    set(glfw3_LIB ${AT_REQ_GLFW3_BASE}/lib/glfw3.lib)
elseif(MINGW)
    set(glfw3_LIB ${AT_REQ_GLFW3_BASE}/lib/libglfw3.a)
endif()
find_package(glfw3 3.3 REQUIRED)


set(FREETYPE_LIBRARY ${AT_REQ_FREETYPE_BASE}/lib/freetype.lib)
set(FREETYPE_INCLUDE_DIRS ${AT_REQ_FREETYPE_BASE}/include)
find_package(freetype REQUIRED)

find_package(OpenCV REQUIRED PATHS ${AT_REQ_OPENCV_INSTALL} )

include_directories(${AT_REQ_GLFW3_BASE}/include)
include_directories(${AT_REQ_FREETYPE_BASE}/include/freetype2)
include_directories(${AT_REQ_OPENCV_INSTALL}/include)
include_directories("imgui2/")
include_directories(${AT_REQ_CUDA_INCLUDE})
include_directories(${AT_REQ_CUDACRT_INCLUDE})

file(GLOB_RECURSE SOURCE_0 "include/*.h")
file(GLOB_RECURSE SOURCE_1 "src/*.cpp")
file(GLOB_RECURSE SOURCE_2 "include/external_ref/*.h")

file(GLOB_RECURSE SOURCE_IMGUI "imgui2/*.h")
file(GLOB_RECURSE SOURCE_IMGUI_2 "imgui2/*.cpp")


file(GLOB SOURCE_STAGES "demo/*.cpp")
file(GLOB SOURCE_STAGES_CUDA "demo/*.cu")

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

foreach( SRC_FILE_NAME ${SOURCE_STAGES} )
    get_filename_component( SRC_FILE_NAME_SHORT ${SRC_FILE_NAME} NAME_WE )
    add_executable( ${SRC_FILE_NAME_SHORT} ${SRC_FILE_NAME} ${SOURCE_0} ${SOURCE_1} ${SOURCE_IMGUI} ${SOURCE_IMGUI_2})
    target_link_libraries( ${SRC_FILE_NAME_SHORT} Vulkan::Vulkan ${glfw3_LIB} ${FREETYPE_LIBRARY} ${OpenCV_LIBS})
endforeach( SRC_FILE_NAME ${SOURCE_STAGES} )

foreach( SRC_FILE_NAME ${SOURCE_STAGES_CUDA} )
    get_filename_component( SRC_FILE_NAME_SHORT ${SRC_FILE_NAME} NAME_WE )

    add_executable( ${SRC_FILE_NAME_SHORT} ${SRC_FILE_NAME} ${SOURCE_0} ${SOURCE_1} ${SOURCE_IMGUI} ${SOURCE_IMGUI_2})
    set_property(TARGET ${SRC_FILE_NAME_SHORT} PROPERTY CUDA_ARCHITECTURES  75)
    set_property(TARGET ${SRC_FILE_NAME_SHORT} PROPERTY CXX_STANDARD 20)
    target_compile_definitions(${SRC_FILE_NAME_SHORT} PUBLIC AT_FEATURE_CUDA_ENABLED)
    target_link_libraries( ${SRC_FILE_NAME_SHORT} Vulkan::Vulkan ${glfw3_LIB} ${FREETYPE_LIBRARY} ${OpenCV_LIBS} cuda)
endforeach( SRC_FILE_NAME ${SOURCE_STAGES_CUDA} )
