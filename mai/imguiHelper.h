#pragma once
#include "mai_config.h"
#include "mai_vk.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"

struct ImGuiRenderer {
  ImGuiRenderer(MAI::Renderer *ren, GLFWwindow *win,
                VkFormat format = VK_FORMAT_UNDEFINED, float fontSize = 24.0f);
  ~ImGuiRenderer();

  void beginFrame(const MAI::Dimissions &dim);
  void endFrame(MAI::CommandBuffer *buff);

  void setDisplayScale(float displayScale);

private:
  void createPipeline();

  MAI::Renderer *ren_ = nullptr;
  GLFWwindow *window = nullptr;
  MAI::Pipeline *pipeline_ = nullptr;
  struct ImGuiRendererImpl *pimpl_ = nullptr;
  VkFormat format;
  float displayScale = 1.0f;
  float fontSize;
  uint32_t frameIndex = 0;
  struct DrawableData {
    MAI::Buffer *vb_ = nullptr;
    MAI::Buffer *ib_ = nullptr;
    uint32_t numAllocateIndices_ = 0;
    uint32_t numAllocateVertices_ = 0;
  };
  DrawableData drawables_[MAX_FRAMES_IN_FLIGHT] = {};
};
