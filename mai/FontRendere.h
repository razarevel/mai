#pragma once

#include "mai_config.h"
#include "mai_vk.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <unordered_map>

struct Vertex {
  glm::vec2 pos;
  glm::vec2 uv;
};

struct Glyph {
  int codepoint;
  int x, y;
  int width, height;
  int x_offset, y_offset;
  int x_advance;
  int page_id;
};

struct Kerning {
  int first;
  int second;
  int ammount;
};

struct FontRenderer {
  FontRenderer(MAI::Renderer *ren, uint32_t width, uint32_t height);
  ~FontRenderer();

  void setText(const char *text, glm::vec2 pos);
  void draw(MAI::CommandBuffer *buff);

private:
  uint32_t screenWidht;
  uint32_t screenHeight;
  MAI::Renderer *ren_;
  MAI::Pipeline *pipeline_;
  MAI::Shader *vert_;
  MAI::Shader *frag_;
  MAI::Texture *texture;
  MAI::Buffer *buffer_ = nullptr;

  std::vector<Vertex> vertices;

  std::unordered_map<uint32_t, Glyph> font_glyphs;
  std::unordered_map<uint32_t, Kerning> font_kernings;

  int atlastWidth;
  int atlastHeight;

  void loadFonts();
  void loadResources();
};
