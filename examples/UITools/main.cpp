#include <iostream>
#include <uiTools.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

using glm::mat4;
using glm::vec3;
using glm::vec4;

int main() {
  MAI::WindowInfo info{
      .width = 1200,
      .height = 800,
      .appName = "UI Tools",
  };
  GLFWwindow *window = MAI::initWindow(info);
  MAI::Renderer *ren = MAI::initVulkanWithSwapChain(window, info.appName);
  UITools *tools = new UITools(ren, window);

  tools->addButtion({
      .position = glm::vec2(0.0f, 0.0f),
  });

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    MAI::CommandBuffer *buff = ren->acquireCommandBuffer();
    buff->cmdBeginRendering({});
    {
      tools->draw(buff);
    }
    buff->cmdEndRendering();
    ren->submit();
  }
  ren->waitDeviceIdle();

  delete tools;

  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
}
