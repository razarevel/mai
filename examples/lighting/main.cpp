#include "maiApp.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

int main() {
  MaiApp *mai = new MaiApp();

  MAI::Shader *vertShader =
      mai->ren->createShader(CURRENT_DIR "examples/2.GLM/main.vert");
  MAI::Shader *fragShader =
      mai->ren->createShader(CURRENT_DIR "examples/2.GLM/main.frag");

  MAI::Pipeline *pipeline = mai->ren->createPipeline({
      .vert = vertShader,
      .frag = fragShader,
      .cullMode = MAI::CullMode::Back,
  });

  const uint32_t isWired = 1;
  MAI::Pipeline *pipelineWired = mai->ren->createPipeline({
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

  auto beforeDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                        uint32_t height, float ratio, float deltaSecond) {};

  auto afterDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                       uint32_t height, float ratio, float deltaSecond) {};

  auto drawDraw = [&](MAI::CommandBuffer *buff, uint32_t width, uint32_t height,
                      float ratio, float deltaSecond) {
    glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);
    p[1][1] *= -1;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0, 2.0f));
    glm::mat4 mvp = p * mai->camera->getViewMatrix() * model;

    // first
    buff->bindPipeline(pipeline);
    buff->cmdPushConstant(&mvp);
    buff->cmdDraw(36);

    // second
    buff->bindPipeline(pipelineWired);
    buff->cmdPushConstant(&mvp);
    buff->cmdDraw(36);
  };

  mai->run(drawDraw, beforeDraw, afterDraw);

  delete pipelineWired;
  delete pipeline;
  delete vertShader;
  delete fragShader;

  delete mai;
  return 0;
}
