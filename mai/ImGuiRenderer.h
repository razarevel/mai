#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "mai_config.h"
#include "mai_vk.h"

struct GuiRenderer {
  GuiRenderer(MAI::Renderer *ren, GLFWwindow *win,
              VkFormat format = VK_FORMAT_UNDEFINED);
  ~GuiRenderer();

  void render(VkCommandBuffer buffer);

private:
  VkFormat format;
  MAI::Renderer *ren_;
  GLFWwindow *window;
  int m_framebufferWidth = 0;
  int m_framebufferHeight = 0;
  MAI::Descriptor *descriptor = nullptr;

  void initGui();
  void createDescriptorPool();
};
