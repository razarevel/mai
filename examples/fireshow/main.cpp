#include "mai_config.h"
#include "mai_vk.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstdio>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

const double kAnimationFPS = 50.0f;
const uint32_t kNumFlipBooks = 3;
const uint32_t kNumFlipBookFrames = 100;

struct AnimateState {
  glm::vec2 position = glm::vec2(0.0f);
  double startTime = 0;
  float time = 0;
  uint32_t textureIndex = 0;
  uint32_t firstFrame = 0;
};

std::vector<AnimateState> g_Animations;

void updateAnimation(float time);

int main() {
  MAI::WindowInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Fire work",
  };
  GLFWwindow *window = MAI::initWindow(info);
  MAI::Renderer *ren = MAI::initVulkanWithSwapChain(window, info.appName);

  std::vector<MAI::Texture *> textures;
  textures.reserve(kNumFlipBooks * kNumFlipBookFrames);

  for (uint32_t book = 0; book != kNumFlipBooks; book++)
    for (uint32_t frame = 0; frame != kNumFlipBookFrames; frame++) {
      char fname[1024];
      snprintf(fname, sizeof(fname),
               RESOURCES_PATH "explosion0%01u/explosion%02u-frame%03u.tga",
               book, book, frame + 1);
      int w, h, comp;
      const stbi_uc *pixels = stbi_load(fname, &w, &h, &comp, 4);
      assert(pixels);
      textures.emplace_back(ren->createImage({
          .type = MAI::TextureType_2D,
          .format = MAI::Format_RGBA_S8,
          .dimensions = {(uint32_t)w, (uint32_t)h},
          .data = pixels,
          .usage = MAI::Sampled_Bit,
      }));
      stbi_image_free((void *)pixels);
    }

  MAI::Shader *vert =
      ren->createShader(CURRENT_DIR "examples/fireshow/main.vert");
  MAI::Shader *frag =
      ren->createShader(CURRENT_DIR "examples/fireshow/main.frag");
  MAI::Pipeline *pipeline = ren->createPipeline({
      .vert = vert,
      .frag = frag,
      .color =
          {
              .blendEnable = true,
              .srcColorBlend = MAI::Src_Alpha,
              .dstColorBlend = MAI::Minus_Src_Alpha,
          },
      .topology = MAI::PrimitiveTopology::Triangle_Strip,
  });
  delete vert;
  delete frag;

  glfwSetMouseButtonCallback(
      window, [](GLFWwindow *window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
          double xpos, ypos;
          int w, h;
          glfwGetWindowSize(window, &w, &h);
          glfwGetCursorPos(window, &xpos, &ypos);
          float x = static_cast<float>(xpos / w);
          float y = static_cast<float>(ypos / h);
          g_Animations.push_back(AnimateState{
              .position = glm::vec2(x, y),
              .startTime = glfwGetTime(),
              .textureIndex = 0,
              .firstFrame = (uint32_t)(rand() % kNumFlipBooks),
          });
        }
      });

  g_Animations.push_back(AnimateState{
      .position = glm::vec2(0.5f, 0.5f),
      .startTime = glfwGetTime(),
      .textureIndex = 0,
      .firstFrame = (uint32_t)(rand() & kNumFlipBooks),
  });

  float deltaSecond = 0.0f;
  double timeStamp = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    const double newTimeStamp = glfwGetTime();
    deltaSecond = static_cast<float>(newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;
    updateAnimation(deltaSecond);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;

    const float ratio = width / (float)height;
    auto easing = [](float t) -> float {
      const float p1 = 0.1f;
      const float p2 = 0.8f;
      if (t <= p1)
        return glm::smoothstep(0.0f, 1.0f, t / p1);
      if (t >= p2)
        return glm::smoothstep(1.0f, 0.0f, (t - p2) / (1.0f - p2));

      return 1.0f;
    };

    MAI::CommandBuffer *buf = ren->acquireCommandBuffer();
    buf->cmdBeginRendering({});
    {
      struct PushConstant {
        glm::mat4 proj;
        uint32_t textureId;
        glm::vec2 pos;
        glm::vec2 size;
        float alphaScale;
      } pc;

      buf->bindPipeline(pipeline);
      for (const AnimateState &s : g_Animations) {
        const float t = s.time / (kNumFlipBookFrames / kAnimationFPS);
        pc = {
            .proj = glm::ortho(0.0f, float(width), 0.0f, float(height)),
            .textureId = textures[s.textureIndex]->getIndex(),
            .pos = s.position * glm::vec2(width, height),
            .size = glm::vec2(height * 0.5f),
            .alphaScale = easing(t),
        };
        buf->cmdPushConstant(&pc);
        buf->cmdDraw(4);
      }
    }
    buf->cmdEndRendering();
    ren->submit();
    delete buf;
  }

  ren->waitDeviceIdle();

  for (auto &it : textures)
    delete it;

  delete pipeline;
  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
}

void updateAnimation(float deltaSecond) {
  for (size_t i = 0; i < g_Animations.size();) {
    g_Animations[i].time += deltaSecond;

    g_Animations[i].textureIndex =
        g_Animations[i].firstFrame +
        (uint32_t)(kAnimationFPS * g_Animations[i].time);

    uint32_t frame = (uint32_t)(kAnimationFPS * g_Animations[i].time);
    if (frame >= kNumFlipBookFrames) {
      g_Animations.erase(g_Animations.begin() + i);
      continue;
    }
    i++;
  }
}
