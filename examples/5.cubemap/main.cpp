#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "Bitmaps.h"
#include "UtilsCubemap.h"
#include "maiApp.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using glm::mat4;
using glm::vec3;
using glm::vec4;

#include <stb_image.h>

#include "mai_config.h"
#include "mai_vk.h"
int main() {
  MaiApp *mai = new MaiApp({
      .width = 1200,
      .height = 800,
      .appName = "Cubemap",
      .enableDepthTexture = true,
  });

  MAI::Renderer *ren = mai->ren;

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
    const aiVector3D n = mesh->mNormals[i];
    vertices.emplace_back(Vertex{
        .pos = vec3(p.x, p.y, p.z),
        .uv = glm::vec2(t.x, t.y),
    });
  }

  for (size_t i = 0; i < mesh->mNumFaces; i++)
    for (size_t j = 0; j != 3; j++)
      indices.emplace_back(mesh->mFaces[i].mIndices[j]);

  MAI::Buffer *vertBuff = ren->createBuffer({
      .usage = MAI::BufferUsage::VertexBuffer,
      .storage = MAI::StorageType_Device,
      .size = vertices.size() * sizeof(Vertex),
      .data = vertices.data(),
  });
  MAI::Buffer *indexBuff = ren->createBuffer({
      .usage = MAI::BufferUsage::IndexBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(uint32_t) * indices.size(),
      .data = indices.data(),
  });

  MAI::Shader *vert =
      ren->createShader(CURRENT_DIR "examples/5.cubemap/main.vert");
  MAI::Shader *frag =
      ren->createShader(CURRENT_DIR "examples/5.cubemap/main.frag");
  MAI::VertexInput input = {
      .attributes =
          {
              {.location = 0, .format = MAI::Float3, .offset = 0},
              {.location = 1,
               .format = MAI::Float2,
               .offset = offsetof(Vertex, uv)},
          },
      .inputBinding = {.stride = sizeof(Vertex)},
  };
  MAI::Pipeline *pipeline = ren->createPipeline({
      .vertexInput = &input,
      .vert = vert,
      .frag = frag,
      .depthFormat = mai->depthTexture->getDeptFormat(),
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

  MAI::Texture *cubemapTex = nullptr;
  {

    int w, h;
    const float *img =
        stbi_loadf(RESOURCES_PATH "piazza_bologni_1k.hdr", &w, &h, nullptr, 4);
    Bitmap in(w, h, 4, eBitmapFormat_Float, img);
    Bitmap out = convertEquirectangularMapToVerticalCross(in);
    std::cout << "where down" << std::endl;
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
      .depthFormat = mai->depthTexture->getDeptFormat(),
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

  auto beforeDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                        uint32_t height, float ratio, float deltaSecond) {
    mat4 p = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 1000.0f);
    p[1][1] *= -1;
    const mat4 m1 =
        glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(1, 0, 0));
    const mat4 m2 =
        glm::rotate(mat4(1.0f), (float)glfwGetTime(), vec3(0.0f, 1.0f, 0.0f));

    PerFrameData pf{
        .model = m2 * m1,
        .view = mai->camera->getViewMatrix(),
        .proj = p,
        .cameraPos = vec4(mai->camera->getPosition(), 1.0f),
        .tex = 0,
        .texCube = cubemapTex->getIndex(),
    };

    buff->update(buferPerFrame, &pf, sizeof(pf));
  };

  auto after = [&](MAI::CommandBuffer *buff, uint32_t width, uint32_t height,
                   float ratio, float deltaSecond) {};
  auto draw = [&](MAI::CommandBuffer *buff, uint32_t width, uint32_t height,
                  float ratio, float deltaSecond) {
    uint64_t address = ren->gpuAddress(buferPerFrame);
    buff->bindPipeline(pipelineSkybox);
    buff->cmdPushConstant(&address);
    buff->cmdDraw(36);

    buff->bindPipeline(pipeline);
    buff->bindVertexBuffer(0, vertBuff);
    buff->bindIndexBuffer(indexBuff, 0, MAI::Uint32);
    buff->cmdPushConstant(&address);
    buff->cmdDrawIndex(indices.size());
  };

  mai->run(draw, beforeDraw, after);

  delete texture;
  delete pipeline;
  delete vertBuff;
  delete indexBuff;
  delete cubemapTex;
  delete buferPerFrame;
  delete pipelineSkybox;
  delete mai;

  return 0;
}
