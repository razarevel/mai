#include "ImGuiRenderer.h"

GuiRenderer::GuiRenderer(MAI::Renderer *ren, GLFWwindow *win, VkFormat format)
    : ren_(ren), window(win), format(format) {
  glfwGetFramebufferSize(window, &m_framebufferWidth, &m_framebufferHeight);
  createDescriptorPool();
  initGui();
}

void GuiRenderer::createDescriptorPool() {
  std::vector<MAI::PoolSize> poolSize = {
      {MAI::Sampler, 1000},
      {MAI::Combined_Image_Sampler, 1000},
      {MAI::Sampled_Image, 1000},
      {MAI::Storage_Image, 1000},
      {MAI::Uniform_Texel_Buffer, 1000},
      {MAI::Storage_Texel_Buffer, 1000},
      {MAI::Uniform_Buffer, 1000},
      {MAI::Storage_Buffer, 1000},
      {MAI::Uniform_Buffer_Dynamic, 1000},
      {MAI::Storage_Buffer_Dynamic, 1000},
      {MAI::Input_Attachment, 1000},
  };
  descriptor = ren_->createDescriptor({
      .flags = MAI::PoolFlags::Free_Descriptor_Set,
      .poolSize = poolSize,
      .maxSets = 1000 * (uint32_t)poolSize.size(),
      .minimal = true,
  });
}

void GuiRenderer::initGui() {
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  io.DisplaySize.x = (float)m_framebufferWidth;
  io.DisplaySize.y = (float)m_framebufferHeight;

  ImGui::GetStyle().FontScaleMain = 1.5f;
  ImGui::StyleColorsDark();
  bool InstallGLFWCallbacks = true;
  ImGui_ImplGlfw_InitForVulkan(window, InstallGLFWCallbacks);
  MAI::VulkanContext *ctx = ren_->getVulkanContext();

  VkPipelineRenderingCreateInfoKHR pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      .viewMask = 0,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &ctx->swapChainFormat,
      .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
  };
  if ((format & VK_FORMAT_UNDEFINED) == 0)
    pipelineInfo.depthAttachmentFormat = format;

  ImGui_ImplVulkan_InitInfo initinfo = {
      .ApiVersion = VK_API_VERSION_1_3,
      .Instance = ctx->instance,
      .PhysicalDevice = ctx->physicalDevice,
      .Device = ctx->device,
      .QueueFamily = ctx->indices.graphcisFamily.value(),
      .Queue = ctx->graphicsQueue,
      .DescriptorPool = descriptor->getDescriptorPool(),
      .MinImageCount = ctx->minImageCount,
      .ImageCount = MAX_FRAMES_IN_FLIGHT,
      .PipelineCache = nullptr,
      .PipelineInfoMain =
          {
              .RenderPass = nullptr,
              .Subpass = 0,
              .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
              .PipelineRenderingCreateInfo = pipelineInfo,
          },
      .UseDynamicRendering = true,
      .Allocator = nullptr,
  };
  ImGui_ImplVulkan_Init(&initinfo);
}

void GuiRenderer::render(VkCommandBuffer buffer) {

  ImDrawData *data = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(data, buffer);
}

GuiRenderer::~GuiRenderer() {
  delete descriptor;
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
