cmake_minimum_required(VERSION 4.4.5)

# If a C++ compiler not specified, prefer modern LLVM Clang
if(NOT CMAKE_CXX_COMPILER AND NOT DEFINED ENV{CXX})
    find_program(AGE_LLVM_CLANGXX
            NAMES clang++ clang++-22 clang++-21 clang++-20 clang++-19
            HINTS
                /opt/homebrew/opt/llvm/bin
                /usr/local/opt/llvm/bin
                "C:/Program Files/LLVM/bin"
    )
    if(AGE_LLVM_CLANGXX)
        set(CMAKE_CXX_COMPILER "${AGE_LLVM_CLANGXX}")
    endif()
endif()

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
set(AGE_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(AGE_SHADER_OUT "${CMAKE_CURRENT_BINARY_DIR}/shaders")

include(cmake/Shaders.cmake)

# Engine library
add_library(age_engine STATIC
        engine/src/core/log.cpp
        engine/src/gfx/device.cpp
        engine/src/gfx/pipeline.cpp
        engine/src/gfx/renderer.cpp
)

add_library(age::engine ALIAS age_engine)

target_sources(age_engine
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/engine/modules
        FILES
            engine/modules/age/core/log.cppm
            engine/modules/age/core/core.cppm
            engine/modules/age/gfx/device.cppm
            engine/modules/age/gfx/pipeline.cppm
            engine/modules/age/gfx/renderer.cppm
            engine/modules/age/gfx/gfx.cppm
            engine/modules/age/age.cppm
)

target_compile_features(age_engine PUBLIC cxx_std_26)

set_target_properties(age_engine PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

target_link_libraries(age_engine
        PUBLIC
        SDL3::SDL3
)

age_enable_warnings(age_engine)

# Game sandbox
if(aGE_BUILD_GAME)
    add_executable(age_game
            game/main.cpp
    )

    target_link_libraries(age_game PRIVATE age::engine)

    target_compile_definitions(age_game PRIVATE
            AGE_SHADER_BIN_DIR="${AGE_SHADER_OUT}")

    age_enable_warnings(age_game)

    add_dependencies(age_game age_shaders)
endif()

# Tooling: clang-format & clang-tidy
find_program(CLANG_FORMAT_BIN
        NAMES clang-format clang-format-23 clang-format-22 clang-format-21
        HINTS /opt/homebrew/bin /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin
)

find_program(CLANG_TIDY_BIN
        NAMES clang-tidy clang-tidy-22 clang-tidy-21
        HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin
)

if(CLANG_FORMAT_BIN)
    file(GLOB_RECURSE AGE_FORMAT_SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cppm
            ${CMAKE_CURRENT_SOURCE_DIR}/game/*.cpp
    )
    add_custom_target(format
            COMMAND ${CLANG_FORMAT_BIN} -i ${AGE_FORMAT_SOURCES}
            COMMENT "Running clang-format on all C++ sources"
            VERBATIM
    )
endif()

if(CLANG_TIDY_BIN)
    add_custom_target(tidy
            COMMAND ${CLANG_TIDY_BIN}
                -p ${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/game/main.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/core/log.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/gfx/device.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/gfx/pipeline.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/gfx/renderer.cpp
            COMMENT "Running clang-tidy static analysis"
            VERBATIM
    )
endif()
