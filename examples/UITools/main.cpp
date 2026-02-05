#include "FontRendere.h"
#include "maiApp.h"
#include "uiTools.h"
#include <format>
#include <iostream>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

using glm::mat4;
using glm::vec3;
using glm::vec4;

int main() {
  MaiApp *mai = new MaiApp();

  UITools *tools = new UITools(mai->ren, mai->window);
  FontRenderer *fmt =
      new FontRenderer(mai->ren, mai->windowInfo.width, mai->windowInfo.width);

  auto beforeDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                        uint32_t height, float ratio, float deltaSecond) {
    std::string mousePos =
        std::format("MouseX: {}, MouseY: {}", mai->mousePos.x, mai->mousePos.y);
    fmt->drawDynamicText(buff, mousePos.c_str(), "mousepos",
                         glm::vec2(0.0f, 0.01f), glm::vec3(0, 0.98, 0.863));
    tools->setCmdBuff(buff);
  };

  tools->addButton({
      .position = glm::vec2(0.0f, 0.0f),
      .size = {(uint32_t)20, (uint32_t)50},
      .color = glm::vec3(1.0f, 0.0f, 0.0f),
  });

  auto afterDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                       uint32_t height, float ratio,
                       float deltaSecond) { fmt->clearGarbge(); };

  auto draw = [&](MAI::CommandBuffer *buff, uint32_t width, uint32_t height,
                  float ratio, float deltaSecond) {
    fmt->draw(buff);
    tools->draw();
  };

  mai->run(draw, beforeDraw, afterDraw);

  delete fmt;
  delete tools;
  delete mai;
}
