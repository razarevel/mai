#include "mai_config.h"
#include "mai_vk.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

using glm::mat4;
using glm::vec3;
using glm::vec4;

int main() {
  MAI::WindowInfo info{
      .width = 1200,
      .height = 800,
      .appName = "GLM Cube",
  };
  GLFWwindow *window = MAI::initWindow(info);
  MAI::Renderer *ren = MAI::initVulkanWithSwapChain(window, info.appName);
  MAI::Shader *vertShader =
      ren->createShader(CURRENT_DIR "examples/2.GLM/main.vert");
  MAI::Shader *fragShader =
      ren->createShader(CURRENT_DIR "examples/2.GLM/main.frag");

  MAI::Pipeline *pipeline = ren->createPipeline({
      .vert = vertShader,
      .frag = fragShader,
      .cullMode = MAI::CullMode::Back,
  });

  const uint32_t isWired = 1;
  MAI::Pipeline *pipelineWired = ren->createPipeline({
      .vert = vertShader,
      .frag = fragShader,
      .specInfo =
          {
              .enteries = {{.constantID = 0, .size = sizeof(uint32_t)}},
              .data = &isWired,
              .dataSize = sizeof(isWired),
          },
      .cullMode = MAI::CullMode::Back,
      .polygon = MAI::PolygonMode::Line,
  });

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;
    const float ratio = width / (float)height;

    const mat4 m =
        glm::rotate(glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, -3.5f)),
                    (float)glfwGetTime(), vec3(1.0f, 1.0f, 1.0f));
    mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);
    p[1][1] *= -1;

    MAI::CommandBuffer *cmd = ren->acquireCommandBuffer();

    cmd->cmdBeginRendering({});
    {

      glm::mat4 mvp = p * m;
      cmd->bindPipeline(pipeline);
      cmd->cmdPushConstant(&mvp);
      cmd->cmdDraw(36);

      cmd->bindPipeline(pipelineWired);
      cmd->cmdPushConstant(&mvp);
      cmd->cmdDraw(36);
    }
    cmd->cmdEndRendering();
    ren->submit();
    delete cmd;
  }
  ren->waitDeviceIdle();

  delete pipelineWired;
  delete pipeline;
  delete vertShader;
  delete fragShader;
  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
  return 0;
}
