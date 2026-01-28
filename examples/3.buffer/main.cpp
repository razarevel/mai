#include "mai_vk.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

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

  const aiScene *scene = aiImportFile(RESOURCES_PATH "rubber_duck/scene.gltf",
                                      aiProcess_Triangulate);
  assert(scene);
  const aiMesh *mesh = scene->mMeshes[0];

  struct Vertex {
    vec3 pos;
    glm::vec2 uv;
  };

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  vertices.reserve(mesh->mNumVertices);
  indices.reserve(3 * mesh->mNumFaces);

  for (size_t i = 0; i < mesh->mNumVertices; i++) {
    const aiVector3D p = mesh->mVertices[i];
    const aiVector3D t =
        mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f);
    vertices.emplace_back(Vertex{
        .pos = vec3(p.x, p.y, p.z),
        .uv = glm::vec2(t.x, t.y),
    });
  }
  for (size_t i = 0; i < mesh->mNumFaces; i++)
    for (size_t j = 0; j != 3; j++)
      indices.emplace_back(mesh->mFaces[i].mIndices[j]);

  MAI::Buffer *vertexBuffer = ren->createBuffer({
      .usage = MAI::VertexBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(Vertex) * vertices.size(),
      .data = vertices.data(),
  });

  MAI::Buffer *indexBuffer = ren->createBuffer({
      .usage = MAI::IndexBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(uint32_t) * indices.size(),
      .data = indices.data(),
  });

  MAI::Shader *vertShader =
      ren->createShader(CURRENT_DIR "examples/3.buffer/main.vert");
  MAI::Shader *fragShader =
      ren->createShader(CURRENT_DIR "examples/3.buffer/main.frag");

  MAI::VertexInput input = {
      .attributes =
          {
              {.location = 0, .format = MAI::Float3, .offset = 0},
              {.location = 1, .format = MAI::Float2, .offset = sizeof(vec3)},
          },
      .inputBinding = {.stride = sizeof(Vertex)},
  };

  MAI::Pipeline *pipeline = ren->createPipeline({
      .vertexInput = &input,
      .vert = vertShader,
      .frag = fragShader,
      .color =
          {
              .depthFormat = depthTexture->getDeptFormat(),
          },
      .cullMode = MAI::CullMode::Back,
  });
  delete vertShader;
  delete fragShader;

  MAI::Texture *texture;
  {
    int w, h, comp;
    const stbi_uc *pixel =
        stbi_load(RESOURCES_PATH "rubber_duck/textures/Duck_baseColor.png", &w,
                  &h, &comp, 4);
    assert(pixel);
    texture = ren->createImage({
        .type = MAI::TextureType_2D,
        .format = MAI::Format_RGBA_S8,
        .dimensions = {(uint32_t)w, (uint32_t)h},
        .data = pixel,
        .usage = MAI::Sampled_Bit,
    });
    stbi_image_free((void *)pixel);
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;
    const float ratio = width / (float)height;

    const mat4 m = glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(1, 0, 0));
    const mat4 v =
        glm::rotate(glm::translate(mat4(1.0f), vec3(0.0f, -0.5f, -1.5f)),
                    (float)glfwGetTime(), vec3(0.0f, 1.0f, 0.0f));
    mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);
    p[1][1] *= -1;

    struct PushConstant {
      mat4 mvp;
      uint32_t texId;
    } pc{
        .mvp = p * v * m,
        .texId = texture->getIndex(),
    };

    MAI::CommandBuffer *cmd = ren->acquireCommandBuffer();
    cmd->cmdBeginRendering({.texture = depthTexture});
    {
      cmd->bindPipeline(pipeline);
      cmd->bindVertexBuffer(0, vertexBuffer);
      cmd->bindIndexBuffer(indexBuffer, 0, MAI::IndexType::Uint32);
      cmd->cmdBindDepthState({
          .depthWriteEnable = true,
          .compareOp = MAI::CompareOp::Less,
      });
      cmd->cmdPushConstant(&pc);
      cmd->cmdDrawIndex(indices.size());
    }
    cmd->cmdEndRendering();
    ren->submit();
    delete cmd;
  }

  ren->waitDeviceIdle();

  delete texture;
  delete depthTexture;
  delete pipeline;
  delete vertexBuffer;
  delete indexBuffer;
  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;

  return 0;
}
