#pragma once
#include "mai_config.h"
#include "mai_vk.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <unordered_map>

struct Tool {
  uint32_t id;
  glm::vec2 position;
  glm::vec2 size;
  glm::vec3 color;
  uint32_t drawSize;
};

struct UITools {
  UITools(MAI::Renderer *ren, GLFWwindow *window);
  ~UITools();

  void addButtion(const Tool &tool);
  void draw(MAI::CommandBuffer *buff);

private:
  MAI::Renderer *ren_;
  GLFWwindow *window_;

  MAI::Shader *vert_ = nullptr;
  MAI::Shader *frag_ = nullptr;
  MAI::Pipeline *pipeline_ = nullptr;

  uint32_t currTool = 0;

  bool inserted = false;

  std::vector<Tool> tools;
  std::unordered_map<uint32_t, MAI::Buffer *> buffers_;
};
