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

  glfwGetFramebufferSize(window_, &screenWidth, &screenHeight);
}

void UITools::addButton(const Tool &tool) {
  if (inserted)
    return;

  float xpos = tool.position.x;
  float ypos = tool.position.y;
  float width = tool.size.width;
  float height = tool.size.height;

  std::vector<glm::vec3> vertices;
  vertices.reserve(6);

  vertices.push_back(glm::vec3(xpos, ypos, 0.0f));
  vertices.push_back(glm::vec3(xpos + width, ypos, 0.0f));
  vertices.push_back(glm::vec3(xpos + width, ypos + height, 0.0f));
  // triangle 1
  vertices.push_back(glm::vec3(xpos, ypos, 0.0f));
  vertices.push_back(glm::vec3(xpos + width, ypos + height, 0.0f));
  vertices.push_back(glm::vec3(xpos, ypos + height, 0.0f));

  MAI::Buffer *buff = ren_->createBuffer({
      .usage = MAI::StorageBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(glm::vec3) * vertices.size(),
      .data = vertices.data(),
  });
  tools.push_back({
      .id = currTool,
      .color = tool.color,
      .drawSize = (uint32_t)vertices.size(),
  });

  buffers_.insert({currTool, buff});

  currTool++;
}

void UITools::draw() {
  struct PushConstant {
    glm::mat4 proj;
    glm::vec3 colors;
    uint64_t vertices;
  } pc;

  glfwGetFramebufferSize(window_, &screenWidth, &screenHeight);

  const float ratio = screenWidth / (float)screenHeight;
  glm::mat4 p = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight,
                           0.0f, // top-left origin
                           -1.0f, 1.0f);

  for (auto &tool : tools) {

    auto it = buffers_.find(tool.id);
    assert(it != buffers_.end());
    pc = {
        .proj = p,
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
