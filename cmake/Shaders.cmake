find_program(AGE_SLANGC_EXECUTABLE
        NAMES slangc
        HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin"
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

if(NOT AGE_SHADER_DIR OR NOT AGE_SHADER_OUT)
    message(FATAL_ERROR "AGE_SHADER_DIR / AGE_SHADER_OUT must be set before including Shaders.cmake.")
endif()
file(MAKE_DIRECTORY ${AGE_SHADER_OUT})

set(AGE_SHADERS "")

foreach(entry IN ITEMS "triangle:vsMain" "triangle:fsMain")
    string(REPLACE ":" ";" parts "${entry}")
    list(GET parts 0 shader_name)
    list(GET parts 1 entry_point)
    set(src ${AGE_SHADER_DIR}/${shader_name}.slang)

    if (NOT EXISTS ${src})
        message(WARNING "aGE: shader missing, skipping: ${src}")
        continue()
    endif()

    # 1. SPIR-V (.spv) for Vulkan
    set(spv ${AGE_SHADER_OUT}/${shader_name}.${entry_point}.spv)
    add_custom_command(
            OUTPUT  ${spv}
            COMMAND ${AGE_SLANGC_EXECUTABLE} ${src}
            -entry ${entry_point}
            -target spirv
            -profile spirv_1_5
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -o ${spv}
            -g
            $<IF:$<CONFIG:Debug>,-O0,-O3>
            DEPENDS ${src}
            COMMENT "slangc ${shader_name}.slang [${entry_point}] -> ${shader_name}.${entry_point}.spv"
            VERBATIM)
    list(APPEND AGE_SHADERS ${spv})

    # 2. Metal Shading Language (.msl) for Metal
    set(msl ${AGE_SHADER_OUT}/${shader_name}.${entry_point}.msl)
    add_custom_command(
            OUTPUT  ${msl}
            COMMAND ${AGE_SLANGC_EXECUTABLE} ${src}
            -entry ${entry_point}
            -target metal
            -o ${msl}
            -g
            $<IF:$<CONFIG:Debug>,-O0,-O3>
            DEPENDS ${src}
            COMMENT "slangc ${shader_name}.slang [${entry_point}] -> ${shader_name}.${entry_point}.msl"
            VERBATIM)
    list(APPEND AGE_SHADERS ${msl})

    # 3. DXIL (.dxil) for Direct3D 12 (Windows only or when dxcompiler library is available)
    if(WIN32)
        set(dxil ${AGE_SHADER_OUT}/${shader_name}.${entry_point}.dxil)
        add_custom_command(
                OUTPUT  ${dxil}
                COMMAND ${AGE_SLANGC_EXECUTABLE} ${src}
                -entry ${entry_point}
                -target dxil
                -profile sm_6_0
                -o ${dxil}
                -g
                $<IF:$<CONFIG:Debug>,-O0,-O3>
                DEPENDS ${src}
                COMMENT "slangc ${shader_name}.slang [${entry_point}] -> ${shader_name}.${entry_point}.dxil"
                VERBATIM)
        list(APPEND AGE_SHADERS ${dxil})
    endif()
endforeach()

add_custom_target(age_shaders DEPENDS ${AGE_SHADERS})
