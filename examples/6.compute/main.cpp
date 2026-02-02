#include "mai_config.h"
#include "mai_vk.h"
#include <cassert>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

using glm::mat4;
using glm::vec3;
using glm::vec4;

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

  const uint32_t kNumMesh = 3048;

  std::vector<vec4> centers(kNumMesh);
  for (vec4 &p : centers)
    p = vec4(glm::linearRand(-vec3(500.0f), +vec3(500.0f)),
             glm::linearRand(0.0f, 3.14159f));

  MAI::Buffer *bufferPosAngle = ren->createBuffer({
      .usage = MAI::StorageBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(vec4) * centers.size(),
      .data = centers.data(),
  });

  MAI::Buffer *bufferMatrices[] = {
      ren->createBuffer({
          .usage = MAI::StorageBuffer,
          .storage = MAI::StorageType_Device,
          .size = sizeof(mat4) * kNumMesh,
      }),
      ren->createBuffer({
          .usage = MAI::StorageBuffer,
          .storage = MAI::StorageType_Device,
          .size = sizeof(mat4) * kNumMesh,
      }),
  };

  const aiScene *scene = aiImportFile(RESOURCES_PATH "rubber_duck/scene.gltf",
                                      aiProcess_Triangulate);
  assert(scene);
  const aiMesh *mesh = scene->mMeshes[0];

  struct Vertex {
    vec3 pos;
    glm::vec2 uv;
    vec3 n;
  };

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  vertices.reserve(mesh->mNumVertices);
  indices.reserve(3 * mesh->mNumFaces);
  for (size_t i = 0; i != mesh->mNumVertices; i++) {
    const aiVector3D p = mesh->mVertices[i];
    const aiVector3D t =
        mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f);
    const aiVector3D n = mesh->mNormals[i];
    vertices.emplace_back(Vertex{
        .pos = vec3(p.x, p.y, p.z),
        .uv = glm::vec2(t.x, t.y),
        .n = glm::vec3(n.x, n.y, n.z),
    });
  }

  for (size_t i = 0; i < mesh->mNumFaces; i++)
    for (size_t j = 0; j != 3; j++)
      indices.emplace_back(mesh->mFaces[i].mIndices[j]);

  MAI::Buffer *vertexBuffer = ren->createBuffer({
      .usage = MAI::StorageBuffer,
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

  MAI::Shader *comp =
      ren->createShader(CURRENT_DIR "examples/6.compute/main.comp", MAI::Comp);

  MAI::Pipeline *computePipeline = ren->createComputePipeline({
      .comp = comp,
  });
  delete comp;

  MAI::Shader *vert =
      ren->createShader(CURRENT_DIR "examples/6.compute/main.vert");
  MAI::Shader *frag =
      ren->createShader(CURRENT_DIR "examples/6.compute/main.frag");

  MAI::Pipeline *pipeline = ren->createPipeline({
      .vert = vert,
      .frag = frag,
      .depthFormat = depthTexture->getDeptFormat(),
      .cullMode = MAI::CullMode::Back,
  });
  delete vert;
  delete frag;
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

  uint32_t frameId = 0;

  float deltaSecond = 0.0f;
  double timeStamp = glfwGetTime();
  while (!glfwWindowShouldClose(window)) {
    const double newTimeStamp = glfwGetTime();
    deltaSecond = static_cast<float>(newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;

    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;
    const float ratio = width / (float)height;
    glm::mat4 proj = glm::perspective(45.0f, ratio, 0.2f, 1500.0f);
    proj[1][1] *= -1;

    const mat4 view = translate(
        mat4(1.0f),
        vec3(0.0f, 0.0f,
             -1000.0f + 500.0f * (1.0f - cos(-glfwGetTime() * 0.5f))));

    MAI::CommandBuffer *buf = ren->acquireCommandBuffer(false);

    const struct {
      mat4 viewproj;
      uint32_t texturId;
      uint64_t bufferPosAngle;
      uint64_t bufferMatrices;
      uint64_t bufferVertices;
      float time;
    } pc{
        .viewproj = proj * view,
        .texturId = texture->getIndex(),
        .bufferPosAngle = ren->gpuAddress(bufferPosAngle),
        .bufferMatrices = ren->gpuAddress(bufferMatrices[frameId]),
        .bufferVertices = ren->gpuAddress(vertexBuffer),
        .time = (float)glfwGetTime(),
    };

    buf->cmdBeginCompute();
    {
      buf->bindComputePipeline(computePipeline);
      buf->cmdPushConstant(&pc);
      buf->cmdDispatchThreadGroups({.width = kNumMesh / 32});
    }
    buf->cmdEndCompute();

    buf->cmdBeginCommandBuffer();
    buf->cmdBeginRendering({.texture = depthTexture});
    {
      buf->bindPipeline(pipeline);
      buf->cmdPushConstant(&pc);
      buf->cmdBindDepthState({
          .depthWriteEnable = true,
          .compareOp = MAI::CompareOp::Less,
      });
      buf->bindIndexBuffer(indexBuffer, 0, MAI::IndexType::Uint32);
      buf->cmdDrawIndex(indices.size(), kNumMesh);
    }
    buf->cmdEndRendering();
    ren->submit(true);
    delete buf;
    frameId = (frameId + 1) & 1;
  }
  ren->waitDeviceIdle();

  delete texture;
  delete bufferMatrices[0];
  delete bufferMatrices[1];
  delete bufferPosAngle;
  delete vertexBuffer;
  delete indexBuffer;
  delete depthTexture;
  delete computePipeline;
  delete pipeline;
  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
}
