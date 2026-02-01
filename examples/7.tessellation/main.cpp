#include "mai_config.h"
#include "mai_vk.h"

#include <assimp/camera.h>
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
  for (size_t i = 0; i != mesh->mNumVertices; i++) {
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

  MAI::Shader *vert =
      ren->createShader(CURRENT_DIR "examples/7.tessellation/main.vert");
  MAI::Shader *frag =
      ren->createShader(CURRENT_DIR "examples/7.tessellation/main.frag");
  MAI::Shader *geom =
      ren->createShader(CURRENT_DIR "examples/7.tessellation/main.gemo");
  MAI::Shader *tesc =
      ren->createShader(CURRENT_DIR "examples/7.tessellation/main.tesc");
  MAI::Shader *tese =
      ren->createShader(CURRENT_DIR "examples/7.tessellation/main.tese");

  MAI::Pipeline *pipeline = ren->createPipeline({
      .vert = vert,
      .frag = frag,
      .geom = geom,
      .tece = tesc,
      .tese = tese,
      .color =
          {
              .depthFormat = depthTexture->getDeptFormat(),
          },
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

  struct BufferPerFrame {
    mat4 model = mat4(1.0f);
    mat4 view = mat4(1.0f);
    mat4 proj = mat4(1.0f);
    vec4 cameraPos;
    uint32_t texture;
    float tesselationScale = 0.5f;
    uint64_t vertices;
  };

  MAI::Buffer *bufferPerFrame = ren->createBuffer({
      .usage = MAI::StorageBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(BufferPerFrame),
  });

  float deltaSecond = 0.0f;
  double timeStamp = glfwGetTime();
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

    MAI::CommandBuffer *buf = ren->acquireCommandBuffer();
    const vec3 cameraPos(0.0f, 1.0f, -1.5f);
    const BufferPerFrame pc = {
        .model = m,
        .view = v,
        .proj = p,
        .cameraPos = vec4(cameraPos, 1.0f),
        .texture = texture->getIndex(),
        .vertices = ren->gpuAddress(vertexBuffer),
    };
    buf->update(bufferPerFrame, &pc, sizeof(pc));
    buf->cmdBeginRendering({.texture = depthTexture});
    {
      uint64_t pcAddress = ren->gpuAddress(bufferPerFrame);
      buf->bindIndexBuffer(indexBuffer, 0, MAI::Uint32);
      buf->bindPipeline(pipeline);
      buf->cmdPushConstant(&pcAddress);
      buf->cmdBindDepthState({
          .depthWriteEnable = true,
          .compareOp = MAI::Less,
      });
      buf->cmdDrawIndex(indices.size());
    }
    buf->cmdEndRendering();
    ren->submit();
    delete buf;
  }
  ren->waitDeviceIdle();

  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
}
