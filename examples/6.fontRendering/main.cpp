#include "FontRenderer.h"
#define MAI_USE_VMA
#include "mai_vk.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <iostream>

struct BufferPerFrame {
  glm::mat4 mvp;
  glm::vec3 color;
  glm::vec2 vertices;
  uint32_t tex;
};

int main() {
  MAI::WindowInfo info{
      .width = 1200,
      .height = 800,
      .appName = "Font Renderering",
  };
  GLFWwindow *window = MAI::initWindow(info);
  MAI::Renderer *ren = MAI::initVulkanWithSwapChain(window, info.appName);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;
    MAI::CommandBuffer *cmd = ren->acquireCommandBuffer();
    const float ratio = width / (float)height;
    cmd->cmdBeginRendering({});
    {
    }
    cmd->cmdEndRendering();
    ren->submit();
    delete cmd;
  }
  ren->waitDeviceIdle();

  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
}
