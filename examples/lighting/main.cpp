#include "imguiHelper.h"

#include "ImGuizmo.h"
#include "imgui.h"

#include "maiApp.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

void EditTransform(float *cameraView, float *cameraProjection, float *matrix) {
  static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
  static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
  if (ImGui::IsKeyPressed(ImGuiKey_T))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  if (ImGui::IsKeyPressed(ImGuiKey_E))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  if (ImGui::IsKeyPressed(ImGuiKey_R))
    mCurrentGizmoOperation = ImGuizmo::SCALE;
  if (ImGui::RadioButton("Translate",
                         mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  ImGui::SameLine();
  if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  ImGui::SameLine();
  if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;

  float matrixTranslate[3], matrixRotation[3], matrixScale[3];
  ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslate, matrixRotation,
                                        matrixScale);
  ImGui::InputFloat3("Tr", matrixTranslate);
  ImGui::InputFloat3("Rt", matrixRotation);
  ImGui::InputFloat3("Sc", matrixScale);
  ImGuizmo::RecomposeMatrixFromComponents(matrixTranslate, matrixRotation,
                                          matrixScale, matrix);

  if (mCurrentGizmoOperation != ImGuizmo::SCALE) {
    if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
      mCurrentGizmoMode = ImGuizmo::LOCAL;
    ImGui::SameLine();
    if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
      mCurrentGizmoMode = ImGuizmo::WORLD;
  }
  static bool useSnap(false);
  if (ImGui::IsKeyPressed(ImGuiKey_S))
    useSnap = !useSnap;
  ImGui::Checkbox("####useSnap", &useSnap);
  ImGui::SameLine();
  ImGuiIO &io = ImGui::GetIO();
  ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
  ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation,
                       mCurrentGizmoMode, matrix, NULL, NULL);
}

int main() {
  MaiApp *mai = new MaiApp({
      .width = 1200,
      .height = 800,
      .appName = "lighting",
      .enableDepthTexture = false,
  });

  MAI::Shader *vertShader =
      mai->ren->createShader(CURRENT_DIR "examples/2.GLM/main.vert");
  MAI::Shader *fragShader =
      mai->ren->createShader(CURRENT_DIR "examples/2.GLM/main.frag");

  MAI::Pipeline *pipeline = mai->ren->createPipeline({
      .vert = vertShader,
      .frag = fragShader,
      .cullMode = MAI::CullMode::Back,
  });

  const uint32_t isWired = 1;
  MAI::Pipeline *pipelineWired = mai->ren->createPipeline({
      .vert = vertShader,
      .frag = fragShader,
      .specInfo =
          {
              .enteries = {{.constantID = 0, .size = sizeof(uint32_t)}},
              .data = &isWired,
              .dataSize = sizeof(isWired),
          },
      .cullMode = MAI::CullMode::Back,
      .polygon = MAI::PolygonMode::Line,
  });

  ImGuiRenderer *imguiRen = new ImGuiRenderer(mai->ren, mai->window);

  auto beforeDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                        uint32_t height, float ratio, float deltaSecond) {};

  auto afterDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                       uint32_t height, float ratio, float deltaSecond) {};

  auto drawDraw = [&](MAI::CommandBuffer *buff, uint32_t width, uint32_t height,
                      float ratio, float deltaSecond) {
    glm::mat4 p = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 1000.0f);
    p[1][1] *= -1;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0, 2.0f));
    glm::mat4 view = mai->camera->getViewMatrix();
    glm::mat4 mvp = p * view * model;

    // first
    buff->bindPipeline(pipeline);
    buff->cmdPushConstant(&mvp);
    buff->cmdDraw(36);

    // second
    buff->bindPipeline(pipelineWired);
    buff->cmdPushConstant(&mvp);
    buff->cmdDraw(36);

    imguiRen->beginFrame({
        .width = width,
        .height = height,
    });
    ImGui::Begin("Viewport");
    float *cameraView = glm::value_ptr(view);
    float *camerProj = glm::value_ptr(p);
    float *matrix = glm::value_ptr(model);
    EditTransform(cameraView, camerProj, matrix);
    ImGui::End();
    imguiRen->endFrame(buff);
  };

  mai->run(drawDraw, beforeDraw, afterDraw);

  delete imguiRen;
  delete pipelineWired;
  delete pipeline;
  delete vertShader;
  delete fragShader;

  delete mai;
  return 0;
}
