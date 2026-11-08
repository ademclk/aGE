//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: device.cpp
// Module: gfx
//*******************************************************************************
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <array>
#include <format>
#include <string>
#include <string_view>

module age.gfx.device;

import age.core.log;

namespace age {
namespace {

#ifdef NDEBUG
constexpr bool kEnableDebug = false;
#else
constexpr bool kEnableDebug = true;
#endif

constexpr std::array kDepthCandidates{
    SDL_GPU_TEXTUREFORMAT_D32_FLOAT,         SDL_GPU_TEXTUREFORMAT_D24_UNORM, SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
    SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT, SDL_GPU_TEXTUREFORMAT_D16_UNORM,
};

}  // namespace

Device::~Device() { shutdown(); }

bool Device::pick_depth_format() {
    for (const SDL_GPUTextureFormat candidate : kDepthCandidates) {
        if (SDL_GPUTextureSupportsFormat(device_, candidate, SDL_GPU_TEXTURETYPE_2D,
                                         SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
            depth_format_ = candidate;
            log_info("selected depth format: {}", static_cast<int>(depth_format_));
            return true;
        }
    }
    log_error("failed to find a supported depth format for device");
    return false;
}

bool Device::init(SDL_Window* window) {
    window_ = window;

    constexpr SDL_GPUShaderFormat requested_formats =
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_DXIL;

    // Passing nullptr for driver name lets SDL automatically select the best backend.
    device_ = SDL_CreateGPUDevice(requested_formats, kEnableDebug, nullptr);
    if (device_ == nullptr) {
        log_error("SDL_CreateGPUDevice failed: {}", SDL_GetError());
        return false;
    }

    driver_name_ = SDL_GetGPUDeviceDriver(device_);
    shader_formats_ = SDL_GetGPUShaderFormats(device_);

    const SDL_PropertiesID props = SDL_GetGPUDeviceProperties(device_);
    const char* dev_name = SDL_GetStringProperty(props, SDL_PROP_GPU_DEVICE_NAME_STRING, "Unknown GPU");
    const char* drv_info = SDL_GetStringProperty(props, SDL_PROP_GPU_DEVICE_DRIVER_INFO_STRING, "");

    log_info("GPU Device created: {} [Driver: {}] {}", dev_name, driver_name_, drv_info);

    if (!SDL_ClaimWindowForGPUDevice(device_, window_)) {
        log_error("SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());
        shutdown();
        return false;
    }
    window_claimed_ = true;

    // Present mode: try VSYNC or MAILBOX
    SDL_GPUPresentMode present_mode = SDL_GPU_PRESENTMODE_VSYNC;
    if (SDL_WindowSupportsGPUPresentMode(device_, window_, SDL_GPU_PRESENTMODE_MAILBOX)) {
        present_mode = SDL_GPU_PRESENTMODE_MAILBOX;
    }
    SDL_SetGPUSwapchainParameters(device_, window_, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, present_mode);

    swapchain_format_ = SDL_GetGPUSwapchainTextureFormat(device_, window_);
    if (swapchain_format_ == SDL_GPU_TEXTUREFORMAT_INVALID) {
        log_error("invalid swapchain format returned by SDL");
        shutdown();
        return false;
    }

    if (!pick_depth_format()) {
        shutdown();
        return false;
    }

    log_info("device initialized (swapchain format: {}, present mode: {})", static_cast<int>(swapchain_format_),
             static_cast<int>(present_mode));
    return true;
}

void Device::wait_idle() const {
    if (device_ != nullptr) {
        SDL_WaitForGPUIdle(device_);
    }
}

void Device::shutdown() {
    if (device_ != nullptr) {
        SDL_WaitForGPUIdle(device_);

        if (window_claimed_ && window_ != nullptr) {
            SDL_ReleaseWindowFromGPUDevice(device_, window_);
            window_claimed_ = false;
        }

        SDL_DestroyGPUDevice(device_);
        device_ = nullptr;
    }

    window_ = nullptr;
    driver_name_ = "";
    shader_formats_ = SDL_GPU_SHADERFORMAT_INVALID;
    swapchain_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
    depth_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
}

}  // namespace age
