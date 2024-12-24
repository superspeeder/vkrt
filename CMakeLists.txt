cmake_minimum_required(VERSION 3.30)
project(vkrt)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)

FetchContent_Declare(vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.1.0
)

set(SPDLOG_USE_STD_FORMAT ON)

FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x
)

FetchContent_MakeAvailable(glm glfw vma spdlog)

find_package(Vulkan REQUIRED)

option(NEURON_ENABLE_TRACE "Enable Trace Logging" OFF)

add_executable(vkrt src/main.cpp
        src/neuron/engine.cpp
        src/neuron/engine.hpp
        src/neuron/render/render_context.cpp
        src/neuron/render/render_context.hpp
        src/neuron/window.cpp
        src/neuron/window.hpp
        src/neuron/logutil.hpp
        src/neuron/render/command_pool.cpp
        src/neuron/render/command_pool.hpp
        src/neuron/render/shader_module.cpp
        src/neuron/render/shader_module.hpp)

target_include_directories(vkrt PRIVATE src/)
target_link_libraries(vkrt Vulkan::Vulkan glm::glm glfw GPUOpen::VulkanMemoryAllocator spdlog::spdlog)
target_compile_definitions(vkrt PRIVATE -DGLM_ENABLE_EXPERIMENTAL)

if (NEURON_ENABLE_TRACE)
    target_compile_definitions(vkrt PRIVATE -DNEURON_ENABLE_TRACE)
endif()
