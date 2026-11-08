//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: renderer.cpp
// Module: gfx
//*******************************************************************************
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

module age.gfx.renderer;

import age.core.log;

namespace age {

Renderer::~Renderer() { shutdown(); }

void Renderer::destroy_depth_texture() {
    if (depth_texture_ != nullptr && device_ != nullptr) {
        SDL_ReleaseGPUTexture(device_, depth_texture_);
        depth_texture_ = nullptr;
    }
    depth_width_ = 0;
    depth_height_ = 0;
}

bool Renderer::ensure_depth_texture(uint32_t width, uint32_t height) {
    if (depth_texture_ != nullptr && width == depth_width_ && height == depth_height_) {
        return true;
    }

    destroy_depth_texture();

    SDL_GPUTextureCreateInfo create_info{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = depth_format_,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = width,
        .height = height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1,
        .props = 0,
    };

    depth_texture_ = SDL_CreateGPUTexture(device_, &create_info);
    if (depth_texture_ == nullptr) {
        log_error("failed to create depth texture ({}x{}): {}", width, height, SDL_GetError());
        return false;
    }

    depth_width_ = width;
    depth_height_ = height;
    log_info("depth texture resized to {}x{}", width, height);
    return true;
}

bool Renderer::init(const Device& device, const Pipeline& pipeline) {
    device_ = device.handle();
    window_ = device.window();
    pipeline_ = pipeline.handle();
    depth_format_ = device.depth_format();

    int w = 0;
    int h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    if (w > 0 && h > 0) {
        if (!ensure_depth_texture(static_cast<uint32_t>(w), static_cast<uint32_t>(h))) {
            return false;
        }
    }

    log_info("renderer initialized for window {}x{}", w, h);
    return true;
}

FrameResult Renderer::draw_frame() {
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device_);
    if (cmd == nullptr) {
        log_error("SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());
        return FrameResult::lost;
    }

    SDL_GPUTexture* swapchain_texture = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window_, &swapchain_texture, &width, &height)) {
        log_error("SDL_WaitAndAcquireGPUSwapchainTexture failed: {}", SDL_GetError());
        SDL_CancelGPUCommandBuffer(cmd);
        return FrameResult::lost;
    }

    if (swapchain_texture == nullptr || width == 0 || height == 0) {
        // Minimized or hidden window
        SDL_SubmitGPUCommandBuffer(cmd);
        return FrameResult::skipped;
    }

    if (!ensure_depth_texture(width, height)) {
        SDL_CancelGPUCommandBuffer(cmd);
        return FrameResult::lost;
    }

    SDL_GPUColorTargetInfo color_target{
        .texture = swapchain_texture,
        .mip_level = 0,
        .layer_or_depth_plane = 0,
        .clear_color = SDL_FColor{.r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
        .resolve_texture = nullptr,
        .resolve_mip_level = 0,
        .resolve_layer = 0,
        .cycle = false,
        .cycle_resolve_texture = false,
        .padding1 = 0,
        .padding2 = 0,
    };

    SDL_GPUDepthStencilTargetInfo depth_target{
        .texture = depth_texture_,
        .clear_depth = 1.0f,
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_DONT_CARE,
        .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
        .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
        .cycle = true,
        .clear_stencil = 0,
        .mip_level = 0,
        .layer = 0,
    };

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color_target, 1, &depth_target);
    if (pass == nullptr) {
        log_error("SDL_BeginGPURenderPass failed: {}", SDL_GetError());
        SDL_CancelGPUCommandBuffer(cmd);
        return FrameResult::lost;
    }

    SDL_BindGPUGraphicsPipeline(pass, pipeline_);
    SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);

    stats_.draw_calls += 1;
    stats_.vertices += 3;

    SDL_EndGPURenderPass(pass);
    SDL_SubmitGPUCommandBuffer(cmd);

    stats_.frame += 1;
    if (stats_.frame % 120 == 0) {
        log_info("frame {}: {} draw calls, {} vertices ({}x{})", stats_.frame, stats_.draw_calls, stats_.vertices, width,
                 height);
        stats_.draw_calls = 0;
        stats_.vertices = 0;
    }

    return FrameResult::ok;
}

void Renderer::shutdown() {
    destroy_depth_texture();
    device_ = nullptr;
    window_ = nullptr;
    pipeline_ = nullptr;
    depth_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
}

}  // namespace age
