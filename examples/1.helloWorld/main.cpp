#include "mai_config.h"
#include "mai_vk.h"

int main() {
  MAI::WindowInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Hello World",
  };
  GLFWwindow *window = MAI::initWindow(info);
  MAI::Renderer *ren = MAI::initVulkanWithSwapChain(window, info.appName);

  MAI::Shader *vertShader =
      ren->createShader(CURRENT_DIR "examples/1.helloWorld/main.vert");
  MAI::Shader *fragShader =
      ren->createShader(CURRENT_DIR "examples/1.helloWorld/main.frag");

  MAI::Pipeline *pipeline = ren->createPipeline({
      .vert = vertShader,
      .frag = fragShader,
  });

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    MAI::CommandBuffer *cmd = ren->acquireCommandBuffer();

    cmd->cmdBeginRendering({});
    {
      cmd->bindPipeline(pipeline);
      cmd->cmdDraw(3);
    }
    cmd->cmdEndRendering();
    ren->submit();
    delete cmd;
  }
  ren->waitDeviceIdle();

  delete pipeline;
  delete vertShader;
  delete fragShader;
  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
  return 0;
}
