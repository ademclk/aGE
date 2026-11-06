find_program(AGE_SLANGC_EXECUTABLE
        NAMES slangc
        HINTS
        "$ENV{VULKAN_SDK}/bin"
        "$ENV{VULKAN_SDK}/Bin"
        DOC "Path to the Slang shader compiler"
)

if(NOT AGE_SLANGC_EXECUTABLE)
    message(FATAL_ERROR
            "Slang compiler (slangc) not found.\n"
            "Install the Slang SDK or add slangc to PATH.\n"
            "Alternatively, set AGE_SLANGC_EXECUTABLE manually."
    )
endif()

message(STATUS "aGE: Using Slang compiler: ${AGE_SLANGC_EXECUTABLE}")

message(STATUS "Slang compiler: ${SLANGC_EXE}")

set(AGE_SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(AGE_SHADER_OUT ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${AGE_SHADER_OUT})


set(AGE_SHADERS "")
foreach(entry IN ITEMS "triangle:vsMain" "triangle:fsMain") # compiled vertex shader and fragment shader
    string(REPLACE ":" ";" parts "${entry}")
    list(GET parts 0 shader_name)
    list(GET parts 1 entry_point)
    set(src ${AGE_SHADER_DIR}/${shader_name}.slang)
    set(spv ${AGE_SHADER_OUT}/${shader_name}.${entry_point}.spv)


    add_custom_command(
            OUTPUT  ${spv}
            COMMAND ${SLANGC_EXE} ${src}
            -entry ${entry_point}
            -target spirv
            -profile spirv_1_5
            -emit-spirv-directly
            -fvk-use-entrypoint-name        # without this every entry point is renamed as "main"
            -o ${spv}
            -g                              # should always on, lets RenderDoc step through source
            $<IF:$<CONFIG:Debug>,-O0,-O3>   # must never be empty
            DEPENDS ${src}
            COMMENT "slangc ${shader_name}.slang [${entry_point}] -> ${shader_name}.${entry_point}.spv"
            VERBATIM)
    list(APPEND AGE_SHADERS ${spv})
endforeach()

add_custom_target(age_shaders ALL DEPENDS ${AGE_SHADERS})