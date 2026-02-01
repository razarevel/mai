cmake_minimum_required(VERSION 3.23)

project(mai LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MAI_BUILD_STATIC "Build mai as static library" ON)

if (MAI_BUILD_STATIC)
    set(MAI_LIB_TYPE STATIC)
else()
    set(MAI_LIB_TYPE SHARED)
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(mai ${MAI_LIB_TYPE} ${MY_SOURCES})
add_library(mai::mai ALIAS mai)


target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC CURRENT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

target_include_directories(mai
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/mai"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(mai
    PUBLIC
		glfw
		Vulkan::Vulkan
		GPUOpen::VulkanMemoryAllocator
		glslang::glslang 
		glslang::SPIRV 
		assimp::assimp
)

add_subdirectory(examples/1.helloWorld)
add_subdirectory(examples/2.GLM)
add_subdirectory(examples/3.buffer)
add_subdirectory(examples/4.textures)
add_subdirectory(examples/5.cubemap)
add_subdirectory(examples/6.compute)
add_subdirectory(examples/7.tessellation)
