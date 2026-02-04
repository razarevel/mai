#include "FontRendere.h"
#include <iostream>
#include <string>

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

int main() {
  MAI::WindowInfo info = {
      .width = 1200,
      .height = 800,
      .appName = "Font Renderer",
  };
  GLFWwindow *window = MAI::initWindow(info);
  MAI::Renderer *ren = MAI::initVulkanWithSwapChain(window, info.appName);

  FontRenderer *fmt = new FontRenderer(ren, info.width, info.height);
  FPS fps;

  float deltaSecond = 0.0f;
  double timeStamp = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    const double newTimeStamp = glfwGetTime();
    deltaSecond = static_cast<float>(newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;
    fps.tick(deltaSecond);

    std::string currentFPS = "FPS " + std::to_string((uint32_t)fps.currentFPS_);

    glfwPollEvents();
    fmt->setText("Yokoso watashi no soul society", glm::vec2(0.5f, 0.5f));
    MAI::CommandBuffer *buf = ren->acquireCommandBuffer();
    fmt->drawDynamicText(buf, currentFPS.c_str(), "fps", glm::vec2(0.9f, 0.01f),
                         glm::vec3(0, 0.98, 0.863));
    buf->cmdBeginRendering({});
    {
      fmt->draw(buf);
    }
    buf->cmdEndRendering();
    ren->submit();
    delete buf;
    fmt->clearGarbge();
  }
  ren->waitDeviceIdle();

  delete fmt;

  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
}
