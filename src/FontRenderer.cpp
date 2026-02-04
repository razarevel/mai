#include "FontRendere.h"
#include <cassert>
#include <cstdio>
#include <iostream>

#include "stb_image.h"

FontRenderer::FontRenderer(MAI::Renderer *ren, uint32_t width, uint32_t height)
    : ren_(ren), screenWidht(width), screenHeight(height) {
  loadFonts();
  loadResources();
}

void FontRenderer::loadFonts() {
  FILE *file = fopen(RESOURCES_PATH "latin.fnt", "r");
  if (!file) {
    std::cerr << "Faile to read font file" << std::endl;
    assert(false);
  }

  fseek(file, 0L, SEEK_END);
  const size_t bytesinfile = ftell(file);
  fseek(file, 0L, SEEK_SET);

  char *buffers = (char *)alloca(bytesinfile + 1);
  const size_t bytesread = fread(buffers, 1, bytesinfile, file);
  fclose(file);

  buffers[bytesread] = 0;

  std::string code(buffers);
  std::string line;
  uint32_t kerning = 0;
  for (char c : code) {
    line += c;
    if (c == '\n') {
      if (line.find("chars count") != std::string::npos) {
        int count;
        std::sscanf(line.c_str(), "chars count=%d", &count);
        font_glyphs.reserve(count);
      } else if (line.find("kernings count") != std::string::npos) {
        int count;
        std::sscanf(line.c_str(), "kernings count=%d", &count);
        font_kernings.reserve(count);
      } else if (line.find("char id=") != std::string::npos) {
        int id, x, y, width, height, xoffset, yoffset, xadvance, page;
        std::sscanf(line.c_str(),
                    "char id=%d x=%d y=%d width=%d height=%d "
                    "xoffset=%d yoffset=%d xadvance=%d page=%d ",
                    &id, &x, &y, &width, &height, &xoffset, &yoffset, &xadvance,
                    &page);
        font_glyphs.insert(
            {(uint32_t)id,
             Glyph{id, x, y, width, height, xoffset, yoffset, xadvance, page}});
      } else if (line.find("kerning first") != std::string::npos) {
        int first, second, amount;
        std::sscanf(line.c_str(), "kerning first=%d  second=%d amount=%d",
                    &first, &second, &amount);
        font_kernings.insert({kerning, Kerning{first, second, amount}});
        kerning++;
      }
      line = "";
    }
  }
}

void FontRenderer::loadResources() {
  int comp;
  const stbi_uc *pixels = stbi_load(RESOURCES_PATH "latin_0.png", &atlastWidth,
                                    &atlastHeight, &comp, 4);
  assert(pixels);
  texture = ren_->createImage({
      .type = MAI::TextureType_2D,
      .format = MAI::Format_RGBA_S8,
      .dimensions = {(uint32_t)atlastWidth, (uint32_t)atlastHeight},
      .data = pixels,
      .usage = MAI::Sampled_Bit,
  });

  vert_ = ren_->createShader(CURRENT_DIR "shaders/font.vert");
  frag_ = ren_->createShader(CURRENT_DIR "shaders/font.frag");
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

void FontRenderer::populateVertices(const char *text,
                                    std::vector<Vertex> &vertices,
                                    glm::vec2 pos, glm::vec3 color) {
  std::string str = text;
  float penX = screenWidht * pos.x;
  float penY = screenHeight * pos.y;
  for (auto &c : str) {
    auto it = font_glyphs.find(c);
    assert(it != font_glyphs.end());
    Glyph &g = it->second;

    float xpos = penX + g.x_offset;
    float ypos = penY + g.y_offset;
    float u0 = g.x / (float)atlastWidth;
    float v0 = g.y / (float)atlastHeight;
    float u1 = (g.x + g.width) / (float)atlastWidth;
    float v1 = (g.y + g.height) / (float)atlastHeight;

    // triangle 0
    vertices.push_back({glm::vec2(xpos, ypos), glm::vec2(u0, v0), color});
    vertices.push_back(
        {glm::vec2(xpos + g.width, ypos), glm::vec2(u1, v0), color});
    vertices.push_back(
        {glm::vec2(xpos + g.width, ypos + g.height), glm::vec2(u1, v1), color});
    // triangle 1
    vertices.push_back({glm::vec2(xpos, ypos), glm::vec2(u0, v0), color});
    vertices.push_back(
        {glm::vec2(xpos + g.width, ypos + g.height), glm::vec2(u1, v1), color});
    vertices.push_back(
        {glm::vec2(xpos, ypos + g.height), glm::vec2(u0, v1), color});
    penX += g.x_advance;
  }
}

void FontRenderer::setText(const char *text, glm::vec2 pos, glm::vec3 color) {
  if (stopInserting)
    return;
  populateVertices(text, verticesAll, pos, color);
}

void FontRenderer::drawDynamicText(MAI::CommandBuffer *buff, const char *text,
                                   const char *id, glm::vec2 pos,
                                   glm::vec3 color) {
  // generate vertices;
  std::vector<Vertex> vertices2;
  populateVertices(text, vertices2, pos, color);
  MAI::Buffer *vertBuffer = nullptr;

  auto it = dynamicBuffers.find(id);

  if (it == dynamicBuffers.end()) {
    // create
    vertBuffer = ren_->createBuffer({
        .usage = MAI::StorageBuffer,
        .storage = MAI::StorageType_Device,
        .size = sizeof(Vertex) * vertices2.size(),
        .data = vertices2.data(),
    });

    dynamicBuffers.insert({id, DynamicText{vertBuffer, vertices2.size()}});

  } else {
    if (it->second.vertices_size != vertices2.size()) {
      garbeBuffers.push_back(it->second.buffer);
      vertBuffer = ren_->createBuffer({
          .usage = MAI::StorageBuffer,
          .storage = MAI::StorageType_Device,
          .size = sizeof(Vertex) * vertices2.size(),
          .data = vertices2.data(),
      });
      it->second.buffer = vertBuffer;
      it->second.vertices_size = vertices2.size();
    } else {
      vertBuffer = it->second.buffer;

      buff->update(vertBuffer, vertices2.data(),
                   sizeof(Vertex) * vertices2.size());
    }
  }

  assert(vertBuffer && "still empty");
}

void FontRenderer::draw(MAI::CommandBuffer *buff) {
  if (!buffer_) {
    buffer_ = ren_->createBuffer({
        .usage = MAI::StorageBuffer,
        .storage = MAI::StorageType_Device,
        .size = sizeof(Vertex) * verticesAll.size(),
        .data = verticesAll.data(),
    });
  }
  struct PushConstant {
    glm::mat4 proj;
    uint32_t textureId;
    uint64_t vertices;
  } pc = {
      .proj = glm::ortho(0.0f, float(screenWidht), 0.0f, float(screenHeight)),
      .textureId = texture->getIndex(),
      .vertices = ren_->gpuAddress(buffer_),
  };
  buff->bindPipeline(pipeline_);
  buff->cmdPushConstant(&pc);
  buff->cmdDraw(verticesAll.size());

  for (auto &it : dynamicBuffers) {
    pc.vertices = ren_->gpuAddress(it.second.buffer);
    buff->bindPipeline(pipeline_);
    buff->cmdPushConstant(&pc);
    buff->cmdDraw(it.second.vertices_size);
  }

  stopInserting = true;
}

void FontRenderer::clearGarbge() {
  for (auto &it : garbeBuffers)
    delete it;
  garbeBuffers.clear();
}

FontRenderer::~FontRenderer() {
  for (auto &it : dynamicBuffers)
    delete it.second.buffer;
  delete buffer_;
  delete pipeline_;
  delete texture;
}
