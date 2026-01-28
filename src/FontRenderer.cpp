#include "FontRenderer.h"
#include <cassert>
#include <ft2build.h>
#include <iostream>
#include <utility>
#include FT_FREETYPE_H

FontRenderer::FontRenderer(MAI::Renderer *ren, const char *filename,
                           float fontSize)
    : ren(ren) {
  assert(filename);

  // rendering fonts
  FT_Library ft;
  if (FT_Init_FreeType(&ft))
    assert(false);

  FT_Face face;
  if (FT_New_Face(ft, filename, 0, &face))
    assert(false);
  FT_Set_Pixel_Sizes(face, 0, 48);

  int offsetX = 0;
  int offsetY = 0;
  int rowHeight = 0;
  int atlasWidth = 512;
  int atlasHeight = 512;

  std::vector<uint8_t> atlasData;

  for (unsigned char c = 0; c < 128; c++) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cout << "ERROR::FREETYPE: Failed to load Glyph: " << c << std::endl;
      continue;
    }
    FT_Bitmap &bmp = face->glyph->bitmap;

    if (offsetX + bmp.width >= atlasWidth) {
      offsetX = 0;
      offsetY += rowHeight;
      rowHeight += 0;
    }

    Glyph g;
    g.size = {bmp.width, bmp.rows};
    g.bearing = {face->glyph->bitmap_left, face->glyph->bitmap_top};
    g.advance = face->glyph->advance.x;
    g.uv0 = {float(offsetX) / atlasWidth, float(offsetY) / atlasHeight};
    g.uv1 = {float(offsetX + bmp.width) / atlasWidth,
             float(offsetY + bmp.rows) / atlasHeight};

    Characters[c] = g;

    Characters.insert(std::pair<char, Glyph>(c, g));

    offsetX += bmp.width;
    if (bmp.rows > rowHeight)
      rowHeight = bmp.rows;
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
  atlasTexture = ren->createImage({
      .type = MAI::TextureType_2D,
      .format = MAI::Format_R8_UN,
      .dimensions = {(uint32_t)atlasWidth, (uint32_t)atlasHeight},
      .data = atlasData.data(),
      .usage = MAI::Sampled_Bit,
  });

  vert_ = ren->createShader(CURRENT_DIR "examples/6.fontRendering/main.vert");
  frag_ = ren->createShader(CURRENT_DIR "examples/6.fontRendering/main.frag");
}
void FontRenderer::begin(MAI::CommandBuffer *buffer) {
  if (pipeline_ == nullptr) {
    MAI::VertexInput input = {
        .attributes =
            {
                {
                    .location = 0,
                    .format = MAI::Float2,
                    .offset = 0,
                },
                {
                    .location = 1,
                    .format = MAI::Float2,
                    .offset = offsetof(Vertex, uv),
                },
                {
                    .location = 2,
                    .format = MAI::Float3,
                    .offset = offsetof(Vertex, color),
                },
            },
        .inputBinding = {.stride = sizeof(Vertex)},
    };
    pipeline_ = ren->createPipeline({
        .vertexInput = &input,
        .vert = vert_,
        .frag = frag_,
        .color =
            {
                .blendEnable = VK_TRUE,
                .srcRGBBlendFactor = MAI::Src_Alpha,
                .dstRGBBlendFactor = MAI::OneMinus_Dst_Alpha,
            },
    });
    delete vert_;
    delete frag_;
  }
  cmd = buffer;
}

void FontRenderer::setText(std::string text, float x, float y, float scale,
                           glm::vec3 color) {

  std::string::const_iterator c;

  for (c = text.begin(); c != text.end(); c++) {
    Glyph g = Characters[*c];

    float xpos = x + g.bearing.x * scale;
    float ypos = y - (g.size.y - g.bearing.y) * scale;
    float w = g.size.x * scale;
    float h = g.size.y * scale;

    float u0 = g.uv0.x;
    float v0 = g.uv0.y;
    float u1 = g.uv1.x;
    float v1 = g.uv1.y;

    std::vector<Vertex> vertices = {
        // triangle 1
        {{xpos, ypos + h}, {u0, v0}, color},
        {{xpos, ypos}, {u0, v1}, color},
        {{xpos + w, ypos}, {u1, v1}, color},

        // triangle 2
        {{xpos, ypos + h}, {u0, v0}, color},
        {{xpos + w, ypos}, {u1, v1}, color},
        {{xpos + w, ypos + h}, {u1, v0}, color},
    };

    verticesAll.push_back(vertices[0]);
    verticesAll.push_back(vertices[1]);
    verticesAll.push_back(vertices[2]);
    verticesAll.push_back(vertices[3]);
    verticesAll.push_back(vertices[4]);
    verticesAll.push_back(vertices[5]);

    x += (g.advance >> 6) * scale;
  }
}

void FontRenderer::end() {
  if (buffer_ == nullptr) {
    buffer_ = ren->createBuffer({
        .usage = MAI::VertexBuffer,
        .storage = MAI::StorageType_Device,
        .size = sizeof(Vertex) * verticesAll.size(),
        .data = verticesAll.data(),
    });
  }

  cmd->bindPipeline(pipeline_);
  struct PushConstant {
    glm::mat4 mvp;
    uint32_t tex;
  } pc = {
      .mvp = glm::ortho(0.0f, 1200.0f, 0.0f, 800.0f),
      .tex = atlasTexture->getIndex(),
  };

  cmd->bindVertexBuffer(0, buffer_);
  cmd->cmdPushConstant(&pc);
  cmd->cmdDraw(verticesAll.size());
}

FontRenderer::~FontRenderer() {
  delete atlasTexture;
  delete pipeline_;
}
