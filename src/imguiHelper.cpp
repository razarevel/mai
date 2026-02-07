#include "imguiHelper.h"
#include <iostream>

struct ImGuiRendererImpl {
  std::vector<MAI::Texture *> textures_;
};

void ImGuiRenderer::createPipeline() {
  const uint32_t nonLinearColorSpace = 1u;
  MAI::Shader *vert_ = ren_->createShader(CURRENT_DIR "shaders/imgui.vert");
  MAI::Shader *frag_ = ren_->createShader(CURRENT_DIR "shaders/imgui.frag");

  MAI::PipelineInfo info = {
      .vert = vert_,
      .frag = frag_,
      .specInfo =
          {
              .enteries = {{.constantID = 0,
                            .size = sizeof(nonLinearColorSpace)}},
              .data = &nonLinearColorSpace,
              .dataSize = sizeof(nonLinearColorSpace),
          },
      .color =
          {
              .blendEnable = true,
              .srcColorBlend = MAI::Src_Alpha,
              .dstColorBlend = MAI::Minus_Src_Alpha,
          },
      .cullMode = MAI::CullMode::None,
  };
  if ((format & VK_FORMAT_UNDEFINED) == 0) {
    info.depthFormat = format;
  }
  pipeline_ = ren_->createPipeline(info);
  delete vert_;
  delete frag_;
}

void ImGuiRenderer::beginFrame(const MAI::Dimissions &dim) {
  ImGuiIO &io = ImGui::GetIO();
  io.DisplaySize = ImVec2(dim.width / displayScale, dim.height / displayScale);
  io.DisplayFramebufferScale = ImVec2(displayScale, displayScale);
  io.IniFilename = nullptr;

  if (!pipeline_)
    createPipeline();

  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiRenderer::endFrame(MAI::CommandBuffer *buff) {
  ImGui::EndFrame();
  ImGui::Render();

  ImDrawData *dd = ImGui::GetDrawData();

  const float fb_width = dd->DisplaySize.x * dd->FramebufferScale.x;
  const float fb_height = dd->DisplaySize.y * dd->FramebufferScale.y;
  if (fb_width <= 0 || fb_height <= 0 || dd->CmdListsCount == 0)
    return;

  if (dd->Textures)
    for (ImTextureData *tex : *dd->Textures)
      switch (tex->Status) {
      case ImTextureStatus_OK:
        continue;
      case ImTextureStatus_Destroyed:
        continue;
      case ImTextureStatus_WantCreate: {
        MAI::Texture *texture = ren_->createImage({
            .type = MAI::TextureType_2D,
            .format = MAI::Format_RGBA_S8,
            .dimensions = {(uint32_t)tex->Width, (uint32_t)tex->Height},
            .data = tex->Pixels,
            .usage = MAI::Sampled_Bit,
        });
        tex->SetTexID((ImTextureID)texture->getIndex());
        tex->SetStatus(ImTextureStatus_OK);
        pimpl_->textures_.emplace_back(texture);
        continue;
      }
      case ImTextureStatus_WantUpdates:
        std::cerr << "imgui wants to update the texture" << std::endl;
        assert(false);
        continue;
      case ImTextureStatus_WantDestroy:
        std::cerr << "imgui wants to destroy the texture" << std::endl;
        assert(false);
        continue;
      }

  buff->cmdBindDepthState({.depthWriteEnable = false});
  buff->cmdBindViewport({
      .x = 0.0f,
      .y = fb_height,
      .width = fb_width,
      .height = -fb_height,
  });

  const float L = dd->DisplayPos.x;
  const float R = dd->DisplayPos.x + dd->DisplaySize.x;
  const float T = dd->DisplayPos.y;
  const float B = dd->DisplayPos.y + dd->DisplaySize.y;

  const ImVec2 clip_off = dd->DisplayPos;
  const ImVec2 clip_scale = dd->FramebufferScale;

  DrawableData &drawableData = drawables_[frameIndex];
  frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

  if (drawableData.numAllocateIndices_ < dd->TotalIdxCount) {
    drawableData.ib_ = ren_->createBuffer({
        .usage = MAI::IndexBuffer,
        .storage = MAI::HostVisible,
        .size = dd->TotalIdxCount * sizeof(ImDrawIdx),
    });
    drawableData.numAllocateIndices_ = dd->TotalIdxCount;
  }

  if (drawableData.numAllocateVertices_ < dd->TotalVtxCount) {
    drawableData.vb_ = ren_->createBuffer({
        .usage = MAI::StorageBuffer,
        .storage = MAI::HostVisible,
        .size = dd->TotalVtxCount * sizeof(ImDrawVert),
    });
    drawableData.numAllocateVertices_ = dd->TotalVtxCount;
  }

  // upload vertex/ index buffers
  {
    ImDrawVert *vtx = (ImDrawVert *)ren_->getMappedPtr(drawableData.vb_);
    uint16_t *idx = (uint16_t *)ren_->getMappedPtr(drawableData.ib_);
    for (const ImDrawList *cmdList : dd->CmdLists) {
      memcpy(vtx, cmdList->VtxBuffer.Data,
             cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
      memcpy(idx, cmdList->IdxBuffer.Data,
             cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
      vtx += cmdList->VtxBuffer.Size;
      idx += cmdList->IdxBuffer.Size;
    }

    // flush
    ren_->flushMappedMemeory(drawableData.vb_, 0,
                             dd->TotalVtxCount * sizeof(ImDrawVert));

    ren_->flushMappedMemeory(drawableData.ib_, 0,
                             dd->TotalIdxCount * sizeof(ImDrawIdx));
  }
  uint32_t idxoffset = 0;
  uint32_t vtxoffset = 0;

  buff->bindIndexBuffer(drawableData.ib_, 0, MAI::Uint16);
  buff->bindPipeline(pipeline_);

  for (const ImDrawList *cmdList : dd->CmdLists) {
    for (int cmd_i = 0; cmd_i < cmdList->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd cmd = cmdList->CmdBuffer[cmd_i];
      ImVec2 clipMin((cmd.ClipRect.x - clip_off.x) * clip_scale.x,
                     (cmd.ClipRect.y - clip_off.y) * clip_scale.y);
      ImVec2 clipMax((cmd.ClipRect.z - clip_off.x) * clip_scale.x,
                     (cmd.ClipRect.w - clip_off.y) * clip_scale.y);

      // clang-format off
			if (clipMin.x < 0.0f) clipMin.x = 0.0f;
      if (clipMin.y < 0.0f) clipMin.y = 0.0f;
      if (clipMax.x > fb_width ) clipMax.x = fb_width;
      if (clipMax.y > fb_height) clipMax.y = fb_height;
      if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
         continue;
      // clang-format on
      struct VulkanImguiBindData {
        float LRTB[4];
        uint64_t vb = 0;
        uint64_t textureId = 0;
        uint64_t samplerId = 0;
      } bindData{
          .LRTB = {L, R, T, B},
          .vb = ren_->gpuAddress(drawableData.vb_),
          .textureId = cmd.GetTexID(),
          .samplerId = 0,
      };

      buff->cmdPushConstant(&bindData);
      buff->cmdBindScissorRect({int32_t(clipMin.x), int32_t(clipMin.y),
                                uint32_t(clipMax.x - clipMin.x),
                                uint32_t(clipMax.y - clipMin.y)});
      buff->cmdDrawIndex(cmd.ElemCount, 1u, idxoffset + cmd.IdxOffset,
                         int32_t(vtxoffset + cmd.VtxOffset));
    }
    idxoffset += cmdList->IdxBuffer.Size;
    vtxoffset += cmdList->VtxBuffer.Size;
  }
}

ImGuiRenderer::ImGuiRenderer(MAI::Renderer *ren, GLFWwindow *win,
                             VkFormat format, float fontSize)
    : ren_(ren), window(win), fontSize(fontSize), pimpl_(new ImGuiRendererImpl),
      format(format) {
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.BackendRendererName = "imgui-mai";
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
  io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui_ImplGlfw_InitForOther(window, window ? true : false);
}

ImGuiRenderer::~ImGuiRenderer() {
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    delete drawables_[i].vb_;
    delete drawables_[i].ib_;
  }
  for (auto &it : pimpl_->textures_)
    delete it;

  delete pipeline_;
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  delete pimpl_;
}
