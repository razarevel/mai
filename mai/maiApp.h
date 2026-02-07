#pragma once
#include "Camera.h"
#include "mai_config.h"
#include "mai_vk.h"
#include <functional>

using DrawFrameFunc =
    std::function<void(MAI::CommandBuffer *buff, uint32_t width,
                       uint32_t height, float ratio, float deltaSecond)>;
struct FPS {
  FPS(float avgInterval = 0.5f) : avgInterval_(avgInterval) {}
  void tick(float deltaSecond) {
    numFrames++;
    accumulateTime += deltaSecond;
    if (accumulateTime > avgInterval_) {
      currentFPS_ = static_cast<float>(numFrames / accumulateTime);
      numFrames = 0;
      accumulateTime = 0;
    }
  }

public:
  float avgInterval_ = 0.5f;
  uint32_t numFrames = 0;
  double accumulateTime = 0;
  float currentFPS_ = 0.0f;
};

struct MaiAppInfo {
  uint32_t width;
  uint32_t height;
  const char *appName;
  bool enableDepthTexture = true;
  bool defaultDescriptorPool = true;
  bool pipelineCaching = false;
};

struct MaiApp {
  MaiApp(const MaiAppInfo &info);
  ~MaiApp();

  MAI::WindowInfo windowInfo;
  MaiAppInfo info_;
  MAI::Renderer *ren = nullptr;
  GLFWwindow *window = nullptr;
  MAI::Texture *depthTexture = nullptr;
  Camera *camera = nullptr;
  glm::vec2 mousePos;
  float currentFPS;

  void run(DrawFrameFunc drawFrame, DrawFrameFunc beforeDraw,
           DrawFrameFunc afterDraw);

private:
  void setMouseConfig();
  void setKeyboardConfig();
};
