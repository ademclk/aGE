//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: pipeline.cppm
// Module: gfx
//*******************************************************************************
module;

#include <SDL3/SDL_gpu.h>

#include <string_view>

export module age.gfx.pipeline;

import age.gfx.device;

export namespace age {

class Pipeline {
   public:
    Pipeline() = default;
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    bool init(const Device& device, std::string_view vertex_shader_base, std::string_view fragment_shader_base,
              const char* vertex_entry = "vsMain", const char* fragment_entry = "fsMain");

    void shutdown();

    [[nodiscard]] SDL_GPUGraphicsPipeline* handle() const { return pipeline_; }

   private:
    SDL_GPUShader* load_shader(std::string_view base_path, const char* entrypoint, SDL_GPUShaderStage stage) const;

    SDL_GPUDevice* device_ = nullptr;
    SDL_GPUShaderFormat shader_formats_ = SDL_GPU_SHADERFORMAT_INVALID;
    SDL_GPUGraphicsPipeline* pipeline_ = nullptr;
};

}  // namespace age
