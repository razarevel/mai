#include "mai_config.h"
#include "mai_vk.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "Bitmaps.h"
#include "UtilsCubemap.h"

using glm::mat4;
using glm::vec3;
using glm::vec4;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
  MAI::WindowInfo info{
      .width = 1200,
      .height = 800,
      .appName = "GLM Cube",
  };
  GLFWwindow *window = MAI::initWindow(info);
  MAI::Renderer *ren = MAI::initVulkanWithSwapChain(window, info.appName);

  MAI::Texture *depthTexture = ren->createImage({
      .type = MAI::TextureType_2D,
      .format = MAI::Format_Z_F32,
      .dimensions = {(uint32_t)info.width, (uint32_t)info.height},
      .usage = MAI::Attachment_Bit,
  });

  MAI::Texture *cubemapTex = nullptr;
  {
    int w, h;
    const float *img =
        stbi_loadf(RESOURCES_PATH "piazza_bologni_1k.hdr", &w, &h, nullptr, 4);
    Bitmap in(w, h, 4, eBitmapFormat_Float, img);
    Bitmap out = convertEquirectangularMapToVerticalCross(in);
    stbi_image_free((void *)img);

    Bitmap cubemap = convertVerticalCrossToCubeMapFaces(out);

    cubemapTex = ren->createImage({
        .type = MAI::TextureType_Cube,
        .format = MAI::Format_RGBA_F32,
        .dimensions = {(uint32_t)cubemap.w_, (uint32_t)cubemap.h_},
        .data = cubemap.data_.data(),
        .usage = MAI::Sampled_Bit,
    });
  }

  MAI::Shader *skyVert =
      ren->createShader(CURRENT_DIR "examples/5.cubemap/skybox.vert");
  MAI::Shader *skyFrag =
      ren->createShader(CURRENT_DIR "examples/5.cubemap/skybox.frag");
  MAI::Pipeline *pipelineSkybox = ren->createPipeline({
      .vert = skyVert,
      .frag = skyFrag,
      .depthFormat = depthTexture->getDeptFormat(),
  });
  delete skyVert;
  delete skyFrag;

  struct PerFrameData {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    uint32_t tex;
    uint32_t texCube;
  };

  MAI::Buffer *buferPerFrame = ren->createBuffer({
      .usage = MAI::UniformBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(PerFrameData),
  });

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;
    const float ratio = width / (float)height;

    const vec3 cameraPos(0.0f, 1.0f, -1.5f);
    mat4 p = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 1000.0f);
    const mat4 m1 =
        glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(1, 0, 0));
    const mat4 m2 =
        glm::rotate(mat4(1.0f), (float)glfwGetTime(), vec3(0.0f, 1.0f, 0.0f));
    const mat4 v =
        glm::lookAt(cameraPos, vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    p[1][1] *= -1;

    MAI::CommandBuffer *cmd = ren->acquireCommandBuffer();
    PerFrameData pf{
        .model = m2 * m1,
        .view = v,
        .proj = p,
        .cameraPos = vec4(cameraPos, 1.0f),
        .tex = 0,
        .texCube = cubemapTex->getIndex(),
    };
    cmd->update(buferPerFrame, &pf, sizeof(pf));
    cmd->cmdBeginRendering({.texture = depthTexture});
    {
      cmd->bindPipeline(pipelineSkybox);
      uint64_t address = ren->gpuAddress(buferPerFrame);
      cmd->cmdPushConstant(&address);
      cmd->cmdDraw(36);
    }
    cmd->cmdEndRendering();
    ren->submit();
    delete cmd;
  }

  ren->waitDeviceIdle();

  delete cubemapTex;
  delete depthTexture;
  delete buferPerFrame;
  delete pipelineSkybox;
  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;

  return 0;
}
