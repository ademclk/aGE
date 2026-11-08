//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: renderer.cppm
// Module: gfx
//*******************************************************************************
module;

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include <cstdint>

export module age.gfx.renderer;

import age.gfx.device;
import age.gfx.pipeline;

export namespace age {

enum class FrameResult { ok, skipped, lost };
    
struct FrameStats {
    uint32_t draw_calls = 0;
    uint32_t vertices = 0;
    uint32_t frame = 0;
};

class Renderer {
   public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init(const Device& device, const Pipeline& pipeline);
    void shutdown();

    FrameResult draw_frame();

    [[nodiscard]] const FrameStats& stats() const { return stats_; }

   private:
    bool ensure_depth_texture(uint32_t width, uint32_t height);
    void destroy_depth_texture();

    SDL_GPUDevice* device_ = nullptr;
    SDL_Window* window_ = nullptr;
    SDL_GPUGraphicsPipeline* pipeline_ = nullptr;
    SDL_GPUTextureFormat depth_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
    SDL_GPUTexture* depth_texture_ = nullptr;
    uint32_t depth_width_ = 0;
    uint32_t depth_height_ = 0;
    FrameStats stats_{};
};

}  // namespace age
