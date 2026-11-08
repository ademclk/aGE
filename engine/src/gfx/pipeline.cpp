//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: pipeline.cpp
// Module: gfx
//*******************************************************************************
module;

#include <SDL3/SDL_gpu.h>

#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

module age.gfx.pipeline;

import age.core.log;

namespace age {

Pipeline::~Pipeline() { shutdown(); }

SDL_GPUShader* Pipeline::load_shader(std::string_view base_path, const char* entrypoint, SDL_GPUShaderStage stage) const {
    std::string path_str{base_path};
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;

    if ((shader_formats_ & SDL_GPU_SHADERFORMAT_MSL) != 0) {
        path_str += ".msl";
        format = SDL_GPU_SHADERFORMAT_MSL;
    } else if ((shader_formats_ & SDL_GPU_SHADERFORMAT_SPIRV) != 0) {
        path_str += ".spv";
        format = SDL_GPU_SHADERFORMAT_SPIRV;
    } else if ((shader_formats_ & SDL_GPU_SHADERFORMAT_DXIL) != 0) {
        path_str += ".dxil";
        format = SDL_GPU_SHADERFORMAT_DXIL;
    } else {
        log_error("device does not support MSL, SPIRV, or DXIL shader formats (mask: 0x{:x})",
                  static_cast<uint32_t>(shader_formats_));
        return nullptr;
    }

    std::FILE* file = std::fopen(path_str.c_str(), "rb");
    if (file == nullptr) {
        log_error("failed to open shader file: {}", path_str);
        return nullptr;
    }

    std::fseek(file, 0, SEEK_END);
    const long size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    if (size <= 0) {
        log_error("shader file is empty: {}", path_str);
        std::fclose(file);
        return nullptr;
    }

    std::vector<uint8_t> code(static_cast<size_t>(size));
    const size_t bytes_read = std::fread(code.data(), 1, code.size(), file);
    std::fclose(file);

    if (bytes_read != code.size()) {
        log_error("failed to read complete shader (read {} of {} bytes): {}", bytes_read, code.size(), path_str);
        return nullptr;
    }

    SDL_GPUShaderCreateInfo create_info{
        .code_size = code.size(),
        .code = code.data(),
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0,
        .props = 0,
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(device_, &create_info);
    if (shader == nullptr) {
        log_error("SDL_CreateGPUShader failed for '{}': {}", path_str, SDL_GetError());
        return nullptr;
    }

    log_info("loaded shader: {} [stage: {}]", path_str, stage == SDL_GPU_SHADERSTAGE_VERTEX ? "vertex" : "fragment");
    return shader;
}

bool Pipeline::init(const Device& device, std::string_view vertex_shader_base, std::string_view fragment_shader_base,
                    const char* vertex_entry, const char* fragment_entry) {
    device_ = device.handle();
    shader_formats_ = device.shader_formats();

    SDL_GPUShader* vert = load_shader(vertex_shader_base, vertex_entry, SDL_GPU_SHADERSTAGE_VERTEX);
    if (vert == nullptr) {
        return false;
    }

    SDL_GPUShader* frag = load_shader(fragment_shader_base, fragment_entry, SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (frag == nullptr) {
        SDL_ReleaseGPUShader(device_, vert);
        return false;
    }

    SDL_GPUColorTargetDescription color_desc{
        .format = device.swapchain_format(),
        .blend_state =
            {
                .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
                .color_blend_op = SDL_GPU_BLENDOP_ADD,
                .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
                .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                .color_write_mask =
                    SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A,
                .enable_blend = false,
                .enable_color_write_mask = false,
                .padding1 = 0,
                .padding2 = 0,
            },
    };

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info{
        .vertex_shader = vert,
        .fragment_shader = frag,
        .vertex_input_state =
            {
                .vertex_buffer_descriptions = nullptr,
                .num_vertex_buffers = 0,
                .vertex_attributes = nullptr,
                .num_vertex_attributes = 0,
            },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state =
            {
                .fill_mode = SDL_GPU_FILLMODE_FILL,
                .cull_mode = SDL_GPU_CULLMODE_NONE,
                .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
                .depth_bias_constant_factor = 0.0f,
                .depth_bias_clamp = 0.0f,
                .depth_bias_slope_factor = 0.0f,
                .enable_depth_bias = false,
                .enable_depth_clip = true,
                .padding1 = 0,
                .padding2 = 0,
            },
        .multisample_state =
            {
                .sample_count = SDL_GPU_SAMPLECOUNT_1,
                .sample_mask = 0,
                .enable_mask = false,
                .enable_alpha_to_coverage = false,
                .padding2 = 0,
                .padding3 = 0,
            },
        .depth_stencil_state =
            {
                .compare_op = SDL_GPU_COMPAREOP_LESS,
                .back_stencil_state = {},
                .front_stencil_state = {},
                .compare_mask = 0,
                .write_mask = 0,
                .enable_depth_test = true,
                .enable_depth_write = true,
                .enable_stencil_test = false,
                .padding1 = 0,
                .padding2 = 0,
                .padding3 = 0,
            },
        .target_info =
            {
                .color_target_descriptions = &color_desc,
                .num_color_targets = 1,
                .depth_stencil_format = device.depth_format(),
                .has_depth_stencil_target = true,
                .padding1 = 0,
                .padding2 = 0,
                .padding3 = 0,
            },
        .props = 0,
    };

    pipeline_ = SDL_CreateGPUGraphicsPipeline(device_, &pipeline_info);

    // Release shaders now that pipeline creation is complete
    SDL_ReleaseGPUShader(device_, vert);
    SDL_ReleaseGPUShader(device_, frag);

    if (pipeline_ == nullptr) {
        log_error("SDL_CreateGPUGraphicsPipeline failed: {}", SDL_GetError());
        return false;
    }

    log_info("graphics pipeline created");
    return true;
}

void Pipeline::shutdown() {
    if (pipeline_ != nullptr && device_ != nullptr) {
        SDL_ReleaseGPUGraphicsPipeline(device_, pipeline_);
        pipeline_ = nullptr;
    }
    device_ = nullptr;
}

}  // namespace age
