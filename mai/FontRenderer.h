#pragma once

#define MAI_USE_VMA
#include "mai_vk.h"

struct FontRenderer {
  FontRenderer(MAI::Renderer *ren, const char *filename, float fontSize);
  ~FontRenderer();

private:
  MAI::Renderer *ren;
  MAI::Shader *vert_ = nullptr;
  MAI::Shader *frag_ = nullptr;
  MAI::Pipeline *pipeline_ = nullptr;
};
