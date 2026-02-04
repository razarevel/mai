#include "uiTools.h"
#include <iostream>

std::vector<glm::vec3> quad = {
    glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f),
    glm::vec3(0.5f, 0.5f, 0.0f),   glm::vec3(-0.5f, -0.5f, 0.0f),
    glm::vec3(0.5f, 0.5f, 0.0f),   glm::vec3(-0.5f, 0.5f, 0.0f),
};

UITools::UITools(MAI::Renderer *ren, GLFWwindow *window)
    : ren_(ren), window_(window) {
  vert_ = ren_->createShader(CURRENT_DIR "shaders/ui.vert");
  frag_ = ren_->createShader(CURRENT_DIR "shaders/ui.frag");

  pipeline_ = ren_->createPipeline({
      .vert = vert_,
      .frag = frag_,
      .color =
          {
              .blendEnable = true,
              .srcColorBlend = MAI::Src_Alpha,
              .dstColorBlend = MAI::Minus_Src_Alpha,
          },
  });
  delete vert_;
  delete frag_;
}

void UITools::addButtion(const Tool &tool) {
  if (inserted)
    return;

  std::vector<glm::vec3> vertices;

  MAI::Buffer *buff = ren_->createBuffer({
      .usage = MAI::StorageBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(glm::vec3) * quad.size(),
      .data = quad.data(),
  });
  tools.push_back({
      .id = currTool,
      .position = tool.position,
      .size = tool.size,
      .color = tool.color,
      .drawSize = (uint32_t)quad.size(),
  });

  buffers_.insert({currTool, buff});

  currTool++;
}

void UITools::draw(MAI::CommandBuffer *buff) {
  struct PushConstant {
    glm::mat4 proj;
    glm::vec3 colors;
    uint64_t vertices;
  } pc;

  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);

  const float ratio = width / (float)height;
  glm::mat4 p = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
  p[1][1] *= -1;
  glm::mat4 model = glm::mat4(1.0f);

  for (auto &tool : tools) {

    auto it = buffers_.find(tool.id);
    assert(it != buffers_.end());
    pc = {
        .proj = p * model,
        .colors = tool.color,
        .vertices = ren_->gpuAddress(it->second),
    };
    buff->bindPipeline(pipeline_);
    buff->cmdPushConstant(&pc);
    buff->cmdDraw(tool.drawSize);
  }
  inserted = true;
}

UITools::~UITools() {
  for (auto &it : buffers_)
    delete it.second;

  delete pipeline_;
}
