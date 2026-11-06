cmake_minimum_required(VERSION 4.4.5)

project(aGE
        VERSION 0.1.0
        DESCRIPTION "Adem's Game Engine"
        LANGUAGES CXX
)

# Global build settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(aGE_BUILD_GAME "Build the game sandbox executable" ON)

# Build configuration
include(cmake/Dependencies.cmake)
include(cmake/CompilerWarnings.cmake)

# Shader configuration
set(AGE_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders")
set(AGE_SHADER_OUT "${CMAKE_CURRENT_BINARY_DIR}/shaders")

include(cmake/VulkanShaders.cmake)

# Engine library
add_library(age_engine STATIC
        engine/src/core/log.cpp
        engine/src/gfx/device.cpp
        engine/src/gfx/pipeline.cpp
        engine/src/gfx/renderer.cpp
        engine/src/gfx/swapchain.cpp
)

add_library(age::engine ALIAS age_engine)

target_include_directories(age_engine
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(age_engine PUBLIC cxx_std_26)

set_target_properties(age_engine PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

target_link_libraries(age_engine
        PUBLIC
        SDL3::SDL3
        Vulkan::Vulkan
)

age_enable_warnings(age_engine)

# Game sandbox
if(aGE_BUILD_GAME)
    add_executable(age_game
            game/main.cpp
    )

    target_link_libraries(age_game PRIVATE age::engine)

    age_enable_warnings(age_game)

    add_dependencies(age_game age_shaders)
endif()