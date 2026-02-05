#include "mai_config.h"
#include "mai_vk.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <iostream>

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
  MAI::Renderer *ren =
      MAI::initVulkanWithSwapChain(window, info.appName,
                                   {
                                       .defaultDescriptorPool = false,
                                   });
  MAI::Descriptor *descriptor = ren->createDescriptor({
      .flags = MAI::PoolFlags::Update_After_Bind,
      .poolSize =
          {
              {MAI::Sampled_Image, MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT},
              {MAI::Sampler, MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT},
              {MAI::Sampled_Image, MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT},
          },
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .bindingFlags =
          {
              // binding 0 (2D textures)
              MAI::Partially_Bound | MAI::Update_After_bind,
              // binding 1 (sampler)
              MAI::Partially_Bound | MAI::Update_After_bind,
              // binding 2 (cubemap)
              MAI::Partially_Bound | MAI::Update_After_bind,
          },
      .setLayoutBinding =
          {
              {
                  .binding = 0,
                  .desctype = MAI::Sampled_Image,
                  .descCount = MAX_TEXTURES,
                  .stage = MAI::Frag,
              },
              {
                  .binding = 1,
                  .desctype = MAI::Sampler,
                  .descCount = MAX_TEXTURES,
                  .stage = MAI::Frag,
              },
              {
                  .binding = 2,
                  .desctype = MAI::Sampled_Image,
                  .descCount = MAX_TEXTURES,
                  .stage = MAI::Frag,
              },
          },
  });

  MAI::Shader *vert =
      ren->createShader(CURRENT_DIR "examples/4.textures/main.vert");
  MAI::Shader *frag =
      ren->createShader(CURRENT_DIR "examples/4.textures/main.frag");
  MAI::Pipeline *pipeline = ren->createPipeline({
      .vert = vert,
      .frag = frag,
      .setLayout = descriptor->getDescriptorSetLayout(),
      .topology = MAI::Triangle_Strip,
  });
  delete vert;
  delete frag;

  MAI::Texture *texture;
  {
    int w, h, comp;
    const stbi_uc *pixel =
        stbi_load(RESOURCES_PATH "wood.jpg", &w, &h, &comp, 4);
    assert(pixel && "not found");
    texture = ren->createImage({
        .type = MAI::TextureType_2D,
        .format = MAI::Format_RGBA_S8,
        .dimensions = {(uint32_t)w, (uint32_t)h},
        .data = pixel,
        .usage = MAI::Sampled_Bit,
    });
  }

  texture->setTextureIndex(1);

  descriptor->updateDescriptorWrite({
      .imageView = texture->getImageView(),
      .binding = 0,
      .count = 1,
      .descType = MAI::Sampled_Image,
  });

  descriptor->updateDescriptorWrite({
      .sampler = texture->getSampler(),
      .binding = 1,
      .count = 1,
      .descType = MAI::Sampler,
  });

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;
    const float ratio = width / (float)height;

    const mat4 m =
        glm::rotate(mat4(1.0f), (float)glfwGetTime(), vec3(0.0f, 0.0f, 1.0f));
    mat4 p = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    p[1][1] *= -1;

    struct PushConstant {
      mat4 mvp;
      uint32_t texId;
    } pc{
        .mvp = p * m,
        .texId = texture->getIndex(),
    };

    MAI::CommandBuffer *cmd = ren->acquireCommandBuffer();
    cmd->cmdBeginRendering({});
    {
      cmd->bindPipeline(pipeline, descriptor);
      cmd->cmdPushConstant(&pc);
      cmd->cmdDraw(4);
    }
    cmd->cmdEndRendering();
    ren->submit();
    delete cmd;
  }
  ren->waitDeviceIdle();

  delete descriptor;
  delete pipeline;
  delete texture;
  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
  return 0;
}
