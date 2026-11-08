//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: device.cppm
// Module: gfx
//*******************************************************************************
module;

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

export module age.gfx.device;

export namespace age {

class Device {
   public:
    Device() = default;
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    bool init(SDL_Window* window);
    void shutdown();
    void wait_idle() const;

    [[nodiscard]] SDL_GPUDevice* handle() const { return device_; }
    [[nodiscard]] SDL_Window* window() const { return window_; }
    [[nodiscard]] const char* driver_name() const { return driver_name_; }
    [[nodiscard]] SDL_GPUShaderFormat shader_formats() const { return shader_formats_; }
    [[nodiscard]] SDL_GPUTextureFormat swapchain_format() const { return swapchain_format_; }
    [[nodiscard]] SDL_GPUTextureFormat depth_format() const { return depth_format_; }

   private:
    bool pick_depth_format();

    SDL_GPUDevice* device_ = nullptr;
    SDL_Window* window_ = nullptr;
    const char* driver_name_ = "";
    SDL_GPUShaderFormat shader_formats_ = SDL_GPU_SHADERFORMAT_INVALID;
    SDL_GPUTextureFormat swapchain_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
    SDL_GPUTextureFormat depth_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
    bool window_claimed_ = false;
};

}  // namespace age
