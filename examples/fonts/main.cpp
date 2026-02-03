#include "FontRendere.h"

int main() {
  MAI::WindowInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Font Renderer",
  };
  GLFWwindow *window = MAI::initWindow(info);
  MAI::Renderer *ren = MAI::initVulkanWithSwapChain(window, info.appName);

  FontRenderer *fmt = new FontRenderer(ren, info.width, info.height);

  fmt->setText("Yokoso watashi no soul society", glm::vec2(0.5f, 0.5f));

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    MAI::CommandBuffer *buf = ren->acquireCommandBuffer();
    buf->cmdBeginRendering({});
    {
      fmt->draw(buf);
    }
    buf->cmdEndRendering();
    ren->submit();
    delete buf;
  }
  ren->waitDeviceIdle();

  delete fmt;

  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
}
