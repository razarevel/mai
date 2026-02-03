#pragma once

// MAI is a single header file library. I tried make it so it doesn't depend on
// any library. You just in vulkan sdk in linux or window to use it.
// Add #define MAI_IMPLEMENTATION before
// #include "mai_vk.h"
//
// For runtime compile shader add
// #define MAI_INCLUDE_GLSLANG
//
// To use VMA in MAI add
// #define MAI_USE_VMA

#include <cstdint>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>

#ifdef MAI_USE_VMA
#include <vk_mem_alloc.h>
#endif

#ifdef MAI_INCLUDE_GLSLANG
#include <glslang/Include/glslang_c_interface.h>
#endif

#define MAIFlags uint32_t

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t MAX_TEXTURES = 4060;
constexpr uint32_t GENERAL_PUSHCONSTANT_SIZE = 256;

namespace MAI {

enum ShaderStage : uint8_t {
  Vert = 0x01,
  Frag = 0x02,
  Geom = 0x04,
  Tese = 0x08,
  Tece = 0x10,
  Comp = 0x20,
  NONE = 0x40,
};

enum CullMode : uint8_t {
  None = 0x00,
  Front = 0x01,
  Back = 0x02,
  Front_Back = Front | Back, // 0x03
  Flag_Bit = 0x04,
};

enum PrimitiveTopology : uint8_t {
  Point_List = 0x01,
  Line_List = 0x02,
  Line_Strip = 0x03,
  Triangle_List = 0x04,
  Triangle_Strip = 0x8,
  Patch_List = 0x10,
};

enum PolygonMode : uint8_t {
  Fill = 0x01,
  Line = 0x02,
  Point = 0x04,
};

enum VertexFormat : uint8_t {
  Float = 0x01,
  Float2 = 0x02,
  Float3 = 0x04,
  Float4 = 0x08,
};

enum IndexType : uint8_t {
  Uint32,
  Uint16,
};

enum InputRate : uint8_t {
  Vertex = 0x01,
  Instance = 0x02,
};

enum CompareOp : uint8_t {
  Never = 0,
  Less = 1,
  Equal = 2,
  Less_or_Equal = 3,
  Greater = 4,
  Not_Equal = 5,
  Greater_or_Equal = 6,
  Always = 7,
};

enum BufferUsage : uint8_t {
  VertexBuffer = 0x01,
  IndexBuffer = 0x02,
  UniformBuffer = 0x04,
  IndirectBuffer = 0x08,
  StorageBuffer = 0x010,
};

enum BufferStorage : uint8_t {
  StorageType_Device = 0x01,
  HostVisible = 0x02,
};

enum TextureType : uint8_t {
  TextureType_2D = 0x01,
  TextureType_Cube = 0x02,
};

enum TextureFormat : uint8_t {
  Format_Z_F32 = 0x01,
  Format_RGBA_S8 = 0x02,
  Format_RGBA_F32 = 0x04,
};

enum TextureUsage : uint8_t {
  Attachment_Bit = 0x01,
  Sampled_Bit = 0x02,
};

enum SamplerFilter : uint8_t {
  Nearst,
  Linear,
};

enum SamplerMipmap : uint8_t {
  Mode_Neart,
  Mode_Linear,
};

enum SamplerWrap : uint8_t {
  Repeat,
  Mirrored_Repeat,
  Clamp_to_Edge,
  Clamp_to_Border,
  Mirror_Clamp_to_edge,
};

enum BlendFactor : uint8_t {
  Src_Alpha,
  Dst_Alpha,
  Minus_Src_Alpha,
};

struct Renderer {
  Renderer(struct VulkanContext *ctx);
  ~Renderer();

  struct CommandBuffer *acquireCommandBuffer();
  struct Shader *createShader(const char *fileName,
                              ShaderStage stage = ShaderStage::NONE);
  struct Pipeline *createPipeline(const struct PipelineInfo &info);
  struct Pipeline *
  createComputePipeline(const struct ComputePipelineInfo &info);
  struct Buffer *createBuffer(const struct BufferInfo &info);
  struct Texture *createImage(const struct TextureInfo &info);

  void waitDeviceIdle();
  void submit();

  uint64_t gpuAddress(struct Buffer *buffer);

private:
  uint32_t lastTextureCount = -1;
  uint32_t lastCubemapCount = -1;
  struct VulkanContext *ctx = nullptr;
};

struct WindowInfo {
  uint32_t width;
  uint32_t height;
  const char *appName = "";
  bool isFullscreen = false;
  bool allowResize = false;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphcisFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() const {
    return graphcisFamily.has_value() && presentFamily.has_value();
  }
};

struct CommandBuffer {
  CommandBuffer(VulkanContext *ctx);

  void cmdBeginRendering(const struct BeginInfo &info);
  void cmdEndRendering();
  void bindPipeline(Pipeline *pipeline);
  void bindComputePipeline(Pipeline *pipeline);
  void bindVertexBuffer(uint32_t firstBinding, Buffer *buffer,
                        uint32_t offset = 0);
  void bindIndexBuffer(Buffer *buffer, VkDeviceSize offset,
                       IndexType indexType);
  void cmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1,
               uint32_t firstIndex = 0, uint32_t firstIntance = 0);
  void cmdDrawIndex(uint32_t indexCount, uint32_t instanceCount = 1,
                    uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                    uint32_t firstInstance = 0);
  void cmdBindDepthState(const struct DepthState &depthInfo);
  void cmdPushConstant(const void *push,
                       uint32_t size = GENERAL_PUSHCONSTANT_SIZE);
  void cmdDispatchThreadGroups(const struct DispatchThreadInfo &info);

  void update(struct Buffer *buffer, const void *data, size_t size);

  Pipeline *lastBindPipline = nullptr;
  Pipeline *lastBindComputePipeline = nullptr;

private:
  VulkanContext *ctx;
};

struct VulkanContext {
  VulkanContext(GLFWwindow *window, const char *appName);
  ~VulkanContext();

  GLFWwindow *window = nullptr;
  const char *appName;
  uint32_t frameIndex = 0;
  uint32_t imageIndex = 0;

  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkDevice device;

#ifdef MAI_USE_VMA
  VmaAllocator allocator;
#endif

  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkSurfaceKHR surface;
  QueueFamilyIndices indices;
  VkDebugUtilsMessengerEXT debugMessenger;

  VkSwapchainKHR swapChain;
  VkFormat swapChainFormat;
  VkColorSpaceKHR swapChainColoSpace;
  VkExtent2D swapChainExtent;

  VkCommandPool commandPool;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  std::vector<VkSemaphore> imageAvailableSemaphore;
  std::vector<VkSemaphore> computeFinishSemaphore;
  std::vector<VkSemaphore> renderFinishSemaphore;
  std::vector<VkFence> drawFences;

  std::vector<VkCommandBuffer> commandBuffers;

  VkShaderModule createShaderModule(uint32_t codeSize, const void *code);
  VkPipelineLayout createPipelineLayout(uint32_t pushConstantSize);
  VkPipeline createGraphicePipeline(VkGraphicsPipelineCreateInfo &info);

#ifdef MAI_USE_VMA
  void createBuffer(const struct BufferDesc &info, VkBuffer &buffer,
                    VmaAllocation &allocation, VmaAllocationInfo &allocInfo);
  void updateBuffer(VkBufferUsageFlags usage, VkBuffer &buffer,
                    VmaAllocation &alloc, size_t size, const void *data);
  void createImage(const struct ImageDesc &info, VkImage &image,
                   VmaAllocation &alloc);
#else
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);
  void updateBuffer(VkBufferUsageFlags usage, VkBuffer &buffer,
                    VkDeviceMemory &deviceMemory, size_t size,
                    const void *data);
  void createImage(const struct ImageDesc &info, VkImage &image,
                   VkDeviceMemory &imageMemory);
#endif

  void createImageView(const struct ImageViewDesc &info, VkImage &image,
                       VkImageView &view);
  void createSampler(const struct SamplerDesc &samplerInfo, VkSampler &sampler);

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  void acquireSwapChainIndex();
  void recreateSwapChain();

  void transition_image_layout(VkImageAspectFlags imageAspect,
                               VkImageLayout oldLayout, VkImageLayout newLayout,
                               VkAccessFlagBits2 srcAccessMask,
                               VkAccessFlagBits2 dstAccessMask,
                               VkPipelineStageFlags2 srcStageMask,
                               VkPipelineStageFlags2 dstStageMask,
                               VkImage &image);

  VkCommandBuffer beginSingleCommandBuffer();
  void endSingleCommandBuffer(VkCommandBuffer commandBuffer);

  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout,
                             uint32_t layerCount);
  void copyBuffeToImage(VkBuffer buffer, VkImage image, uint32_t width,
                        uint32_t height, bool isCube);

  void updateDescriptorImageWrite(VkImageView imageView, VkSampler sampler,
                                  uint32_t imageIndex, bool isCubemap = false);

private:
  void createInstance();
  void setupDebugger();
  void createSurfaceKHR();
  void pickPhysicalDevice();
  void createLogicalDevice();

#ifdef MAI_USE_VMA
  void createVmaAllocation();
#endif

  void createSwapChain();
  void createSwapChainImageViews();
  void cleanupSwapChain();

  void createSyncObj();
  void createCommandBuffer();
  void createCommandPool();

  void createDescriptorPool();
  void createDescriptorSetLayout();
  void createDescriptorSets();

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};

struct BeginInfo {
  float clearColor[4] = {0.05f, 0.05f, 0.05f, 1.0f};
  struct Texture *texture = nullptr;
};

struct DepthState {
  bool depthWriteEnable = false;
  CompareOp compareOp = CompareOp::Always;
};
struct DispatchThreadInfo {
  uint32_t width = 1;
  uint32_t height = 1;
  uint32_t depth = 1;
};

struct VertexAttribute {
  uint32_t binding = 0;
  uint32_t location;
  VertexFormat format;
  uint32_t offset;
};

struct InputBinding {
  uint32_t binding = 0;
  uint32_t stride;
  InputRate input = InputRate::Vertex;
};

struct VertexInput {
  std::vector<VertexAttribute> attributes;
  InputBinding inputBinding;
};

struct SpecialMapEntries {
  uint32_t constantID;
  size_t size;
};

struct SpecInfo {
  std::vector<SpecialMapEntries> enteries;
  const void *data;
  size_t dataSize;
};

struct MaiColorInfo {
  bool blendEnable = false;
  BlendFactor srcColorBlend;
  BlendFactor dstColorBlend;
};

struct PipelineInfo {
  VertexInput *vertexInput;
  Shader *vert;
  Shader *frag;
  Shader *geom;
  Shader *tece;
  Shader *tese;
  SpecInfo specInfo;
  MaiColorInfo color;
  VkFormat depthFormat;
  CullMode cullMode = CullMode::None;
  PrimitiveTopology topology = PrimitiveTopology::Triangle_List;
  PolygonMode polygon = PolygonMode::Fill;
  uint32_t pushConstantSize = GENERAL_PUSHCONSTANT_SIZE;
  uint32_t patchControllPoints;
};

struct ComputePipelineInfo {
  VertexInput *vertexInput;
  Shader *comp;
  SpecInfo specInfo;
  uint32_t pushConstantSize = GENERAL_PUSHCONSTANT_SIZE;
};

struct Shader {
  Shader(VkDevice &device, VkShaderModule sm, ShaderStage stage)
      : device(device), sm_(sm), stage_(stage) {}
  ~Shader() {
    if (sm_ != VK_NULL_HANDLE)
      vkDestroyShaderModule(device, sm_, nullptr);
  };

  VkShaderModule &getShader() { return sm_; }
  ShaderStage getShaderStage() const { return stage_; }

private:
  VkShaderModule sm_ = VK_NULL_HANDLE;
  ShaderStage stage_;
  VkDevice &device;
};

struct BufferInfo {
  MAIFlags usage;
  BufferStorage storage;
  size_t size;
  const void *data = nullptr;
};

struct Dimissions {
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
};

struct TextureInfo {
  TextureType type;
  TextureFormat format;
  Dimissions dimensions;
  const void *data;
  TextureUsage usage;
};

#ifdef MAI_USE_VMA
struct BufferDesc {
  VkDeviceSize size;
  VkBufferUsageFlags usage;
  VmaAllocationCreateFlags allocflags;
  VmaMemoryUsage memoryUsage;
  VkMemoryPropertyFlags propertyFlags;
};
#endif

struct ImageDesc {
  VkImageCreateFlags flags;
  VkImageType type;
  VkExtent3D extent;
  const void *data = nullptr;
  uint32_t mipLevel = 1;
  uint32_t arrayLayers = 1;
  VkFormat format;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
};

struct ImageViewDesc {
  VkFormat format;
  VkImageViewType viewType;
  VkImageAspectFlags aspect;
  uint32_t layerCount = 1;
};

struct SamplerDesc {
  SamplerFilter minFilter = SamplerFilter::Linear;
  SamplerFilter magFilter = SamplerFilter::Linear;
  SamplerMipmap mipMap = SamplerMipmap::Mode_Neart;
  SamplerWrap wrapU = SamplerWrap::Repeat;
  SamplerWrap wrapV = SamplerWrap::Repeat;
  SamplerWrap wrapW = SamplerWrap::Repeat;
  CompareOp depthCompareOp = CompareOp::Always;
  bool depthCompareEnabled = false;
};

struct commandBufferInfo {
  VkCommandBuffer &commandBuffer;
  VkImage &swapChainImage;
  VkImageView &swapChainImageView;
  VkExtent2D &swapChainExtent;
};

#ifdef MAI_USE_VMA
struct Buffer {
  Buffer(VmaAllocator &allocator, VkBuffer buffer, VmaAllocation allocation,
         VmaAllocationInfo allocaInfo, VkBufferUsageFlags flags)
      : allocator(allocator), buf_(buffer), alloc_(allocation),
        allocaInfo_(allocaInfo), usageFlags(flags) {}

  ~Buffer() {
    if (buf_ != VK_NULL_HANDLE)
      vmaDestroyBuffer(allocator, buf_, alloc_);
  }

  VkBuffer &getBuffer() { return buf_; }
  VmaAllocation &getAllocation() { return alloc_; }
  VkBufferUsageFlags getBufferUsage() const { return usageFlags; }

private:
  VmaAllocator &allocator;
  VkBuffer buf_ = VK_NULL_HANDLE;
  VmaAllocation alloc_;
  VmaAllocationInfo allocaInfo_;
  VkBufferUsageFlags usageFlags;
};

#else
struct Buffer {
  Buffer(VkDevice &device, VkBuffer buffer, VkDeviceMemory bufMemory,
         VkBufferUsageFlags flags)
      : device(device), buf_(buffer), bufMem_(bufMemory), usageFlags(flags) {}
  ~Buffer() {
    if (bufMem_ != VK_NULL_HANDLE)
      vkFreeMemory(device, bufMem_, nullptr);
    if (buf_ != VK_NULL_HANDLE)
      vkDestroyBuffer(device, buf_, nullptr);
  }

  VkBuffer &getBuffer() { return buf_; }
  VkDeviceMemory &getBufferMem() { return bufMem_; }
  VkBufferUsageFlags getBufferUsage() const { return usageFlags; }

private:
  VkDevice &device;
  VkBuffer buf_ = VK_NULL_HANDLE;
  VkDeviceMemory bufMem_ = VK_NULL_HANDLE;
  VkBufferUsageFlags usageFlags;
};
#endif

#ifdef MAI_USE_VMA
struct Texture {
  Texture(VkDevice &device, VmaAllocator &allocator, VkImage image,
          VmaAllocation allocation, VkImageView view, VkSampler sampler,
          VkFormat format)
      : device(device), allocator(allocator), alloc_(allocation), image_(image),
        view_(view), sampler_(sampler), format_(format) {}
  ~Texture() {
    if (sampler_ != VK_NULL_HANDLE)
      vkDestroySampler(device, sampler_, nullptr);
    if (view_ != VK_NULL_HANDLE)
      vkDestroyImageView(device, view_, nullptr);
    if (image_ != VK_NULL_HANDLE)
      vmaDestroyImage(allocator, image_, alloc_);
  }

  VkImage &getImage() { return image_; }
  VkImageView &getImageView() { return view_; }
  VkFormat &getDeptFormat() { return format_; }
  VkSampler &getSampler() { return sampler_; }

  void setTextureIndex(uint32_t index) { index_ = index; }
  uint32_t &getIndex() { return index_; }

private:
  VkDevice &device;
  VmaAllocator &allocator;
  VkFormat format_ = VK_FORMAT_UNDEFINED;
  VkImage image_ = VK_NULL_HANDLE;
  VmaAllocation alloc_ = VK_NULL_HANDLE;
  VkImageView view_ = VK_NULL_HANDLE;
  VkSampler sampler_ = VK_NULL_HANDLE;
  uint32_t index_ = -1;
};
#else
struct Texture {
  Texture(VkDevice &device, VkImage image, VkDeviceMemory imageMemory,
          VkImageView view, VkSampler sampler, VkFormat format)
      : device(device), image_(image), memory_(imageMemory), view_(view),
        sampler_(sampler), format_(format) {};
  ~Texture() {
    if (sampler_ != VK_NULL_HANDLE)
      vkDestroySampler(device, sampler_, nullptr);
    if (view_ != VK_NULL_HANDLE)
      vkDestroyImageView(device, view_, nullptr);
    if (image_ != VK_NULL_HANDLE)
      vkDestroyImage(device, image_, nullptr);
    if (memory_ != VK_NULL_HANDLE)
      vkFreeMemory(device, memory_, nullptr);
  }

  VkImage &getImage() { return image_; }
  VkImageView &getImageView() { return view_; }
  VkFormat &getDeptFormat() { return format_; }
  VkSampler &getSampler() { return sampler_; }

  void setTextureIndex(uint32_t index) { index_ = index; }
  uint32_t &getIndex() { return index_; }

private:
  VkDevice &device;
  VkFormat format_;
  VkImage image_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkImageView view_ = VK_NULL_HANDLE;
  VkSampler sampler_ = VK_NULL_HANDLE;
  uint32_t index_ = -1;
};
#endif

struct Pipeline {
  Pipeline(VkDevice &dev, VkPipeline pm, VkPipelineLayout piplayout)
      : device(dev), pipeline_(pm), layout_(piplayout) {};
  ~Pipeline() {
    vkDestroyPipelineLayout(device, layout_, nullptr);
    vkDestroyPipeline(device, pipeline_, nullptr);
  }

  VkPipeline &getPipeline() { return pipeline_; }
  VkPipelineLayout &getPipelineLayout() { return layout_; }

private:
  VkDevice &device;
  VkPipeline pipeline_;
  VkPipelineLayout layout_;
};

GLFWwindow *initWindow(const WindowInfo &info);
Renderer *initVulkanWithSwapChain(GLFWwindow *window = nullptr,
                                  const char *appName = nullptr);

}; // namespace MAI

#ifdef MAI_IMPLEMENTATION
#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace MAI {

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t MAX_TEXTURES = 1024;
constexpr uint32_t GENERAL_PUSHCONSTANT_SIZE = 256;

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> surfaceFormats;
  std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
};

#ifdef _DEBUG
constexpr bool enableValidation = true;
#else
constexpr bool enableValidation = false;
#endif

bool checkValidation();
std::vector<const char *> getRequiredExtensions();
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT type,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallback, void *);
VkResult
CreateDebugUtilsMessengerEXT(VkInstance instance,
                             VkDebugUtilsMessengerCreateInfoEXT *createInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkDebugUtilsMessengerEXT *pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks *pAllocator);

void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

VkSurfaceFormatKHR
chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

VkPresentModeKHR chooseSwapChainPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes);

VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR &capabilities,
                                 GLFWwindow *window);
SwapChainSupportDetails querrySwapChainSupport(VkPhysicalDevice device,
                                               VkSurfaceKHR surface);
void endsWidth();
ShaderStage getShaderStageFromFile(const char *filename);
std::string readShaderFile(const char *filename);
std::vector<char> readShaderBinaryFile(const char *filename);
VkShaderStageFlags getShaderStage(MAIFlags stages);
VkShaderStageFlagBits getShaderStageBits(ShaderStage stage);

VkFormat getVertexFormat(VertexFormat format);
VkVertexInputRate getVertexInputRate(InputRate rate);
VkIndexType getIndexType(IndexType type);
VkCullModeFlags getCullMode(CullMode mode);
VkPolygonMode getPolygonMode(PolygonMode mode);
VkPrimitiveTopology getPrimitiveTopology(PrimitiveTopology topology);
VkBufferUsageFlags getBufferUsageFlags(MAIFlags usages);
VkFormat getFormat(TextureFormat format);
VkImageUsageFlags getImageUsage(TextureUsage usage);
VkFilter getSamplerFilter(SamplerFilter filter);
VkSamplerMipmapMode getSamplerMipmapMode(SamplerMipmap mode);
VkSamplerAddressMode getSamplerWrap(SamplerWrap wrap);
VkCompareOp getCompareOp(CompareOp op);
VkBlendFactor getBlendFactor(BlendFactor factor);

#ifdef MAI_INCLUDE_GLSLANG
void compileShaderGlslang(ShaderStage stage, const char *code,
                          std::vector<uint8_t> *outSPIRV,
                          const glslang_resource_t *glslLangResource);
glslang_resource_t getGLSLangResources(const VkPhysicalDeviceLimits &limits);
#endif

Renderer::Renderer(VulkanContext *ctx) : ctx(ctx) {}

struct CommandBuffer *Renderer::acquireCommandBuffer() {
  ctx->acquireSwapChainIndex();

  VkCommandBuffer &commandBuffer = ctx->commandBuffers[ctx->frameIndex];
  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = 0,
      .pInheritanceInfo = nullptr,
  };

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    throw std::runtime_error("failed to begin command buffer");

  CommandBuffer *cmd = new CommandBuffer(ctx);
  return cmd;
}

struct Shader *Renderer::createShader(const char *filename, ShaderStage stage) {
  VkShaderStageFlagBits shaderStage;
  if (stage == ShaderStage::NONE)
    stage = getShaderStageFromFile(filename);

  std::string str = filename;
  bool isSPV = str.find("spv") != str.npos;

  VkShaderModule sm_;
  if (!isSPV) {
    std::string code = readShaderFile(filename);
#ifdef MAI_INCLUDE_GLSLANG
    std::vector<uint8_t> buffer;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(ctx->physicalDevice, &properties);
    const glslang_resource_t glslangResource =
        getGLSLangResources(properties.limits);

    compileShaderGlslang(stage, code.c_str(), &buffer, &glslangResource);
    sm_ = ctx->createShaderModule((uint32_t)buffer.size(), buffer.data());
#else
    throw std::runtime_error(
        " shader is not compiled, Please provide the compile version of the "
        "shader or define MAI_INCLUDE_GLSLANG");
#endif

  } else {
    std::vector<char> code = readShaderBinaryFile(filename);
    sm_ = ctx->createShaderModule((uint32_t)code.size(), code.data());
  }

  Shader *shader = new Shader(ctx->device, sm_, stage);

  return shader;
}

struct Pipeline *Renderer::createPipeline(const struct PipelineInfo &info_) {
  // preparing shader stages
  std::vector<VkPipelineShaderStageCreateInfo> stages;

  std::vector<VkSpecializationMapEntry> entries;
  const bool hasSpec = !info_.specInfo.enteries.empty();
  VkSpecializationInfo specInfo;

  if (hasSpec) {
    entries.reserve(info_.specInfo.enteries.size());
    uint32_t offset = 0;
    for (auto &spec : info_.specInfo.enteries) {
      entries.emplace_back(VkSpecializationMapEntry{
          .constantID = spec.constantID,
          .offset = offset,
          .size = spec.size,
      });
      offset += spec.size;
    }

    specInfo = {
        .mapEntryCount = static_cast<uint32_t>(entries.size()),
        .pMapEntries = entries.data(),
        .dataSize = info_.specInfo.dataSize,
        .pData = info_.specInfo.data,
    };
  }
  if (info_.vert)
    stages.emplace_back(VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = getShaderStageBits(info_.vert->getShaderStage()),
        .module = info_.vert->getShader(),
        .pName = "main",
        .pSpecializationInfo = hasSpec ? &specInfo : nullptr,
    });

  if (info_.frag)
    stages.emplace_back(VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = getShaderStageBits(info_.frag->getShaderStage()),
        .module = info_.frag->getShader(),
        .pName = "main",
        .pSpecializationInfo = hasSpec ? &specInfo : nullptr,
    });

  if (info_.geom)
    stages.emplace_back(VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = getShaderStageBits(info_.geom->getShaderStage()),
        .module = info_.geom->getShader(),
        .pName = "main",
        .pSpecializationInfo = hasSpec ? &specInfo : nullptr,
    });

  if (info_.tece)
    stages.emplace_back(VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = getShaderStageBits(info_.tece->getShaderStage()),
        .module = info_.tece->getShader(),
        .pName = "main",
        .pSpecializationInfo = hasSpec ? &specInfo : nullptr,
    });

  if (info_.tese)
    stages.emplace_back(VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = getShaderStageBits(info_.tese->getShaderStage()),
        .module = info_.tese->getShader(),
        .pName = "main",
        .pSpecializationInfo = hasSpec ? &specInfo : nullptr,
    });

  std::vector<VkVertexInputAttributeDescription> attributes;
  VkVertexInputBindingDescription binding;

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
  };

  if (info_.vertexInput) {
    attributes.reserve(info_.vertexInput->attributes.size());
    for (VertexAttribute &attribute : info_.vertexInput->attributes)
      attributes.emplace_back(VkVertexInputAttributeDescription{
          .location = attribute.location,
          .binding = attribute.binding,
          .format = getVertexFormat(attribute.format),
          .offset = attribute.offset,
      });

    binding = {
        .binding = info_.vertexInput->inputBinding.binding,
        .stride = info_.vertexInput->inputBinding.stride,
        .inputRate = getVertexInputRate(info_.vertexInput->inputBinding.input),
    };

    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &binding;
  }

  std::vector dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
      VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
  };

  VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data(),
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = getPrimitiveTopology(info_.topology),
      .primitiveRestartEnable = VK_FALSE,
  };

  VkPipelineTessellationStateCreateInfo tesseInfo{};
  if (info_.tece && info_.tese) {
    tesseInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        .patchControlPoints = info_.patchControllPoints,
    };
  }

  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };

  VkPipelineRasterizationStateCreateInfo raserization{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = getPolygonMode(info_.polygon),
      .cullMode = getCullMode(info_.cullMode),
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multiSampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  if (info_.color.blendEnable) {
    colorBlendAttachment.blendEnable = VK_TRUE;

    if (getBlendFactor(info_.color.srcColorBlend) ==
        VK_BLEND_FACTOR_SRC_ALPHA) {
      colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      colorBlendAttachment.dstColorBlendFactor =
          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    }

    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

    if (getBlendFactor(info_.color.dstColorBlend) ==
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA) {
      colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      colorBlendAttachment.dstAlphaBlendFactor =
          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    }

    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  }

  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
  };

  VkPipelineRenderingCreateInfo pipelineRenderCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &ctx->swapChainFormat,
  };

  if (info_.depthFormat)
    pipelineRenderCreateInfo.depthAttachmentFormat = info_.depthFormat;

  VkPipelineDepthStencilStateCreateInfo depthStencil{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_FALSE,
      .depthWriteEnable = VK_FALSE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
  };

  VkPipelineLayout pipelineLayout =
      ctx->createPipelineLayout(info_.pushConstantSize);

  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &pipelineRenderCreateInfo,
      .stageCount = static_cast<uint32_t>(stages.size()),
      .pStages = stages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pTessellationState = (info_.tece && info_.tese) ? &tesseInfo : nullptr,
      .pViewportState = &viewportState,
      .pRasterizationState = &raserization,
      .pMultisampleState = &multiSampling,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = pipelineLayout,
      .renderPass = nullptr,
  };

  VkPipeline pipeline = ctx->createGraphicePipeline(pipelineInfo);

  Pipeline *pipline = new Pipeline(ctx->device, pipeline, pipelineLayout);
  return pipline;
}

struct Pipeline *
Renderer::createComputePipeline(const struct ComputePipelineInfo &info) {
  if (info.comp->getShaderStage() != MAI::Comp) {
    std::cerr << "shader isn't a compute shader " << std::endl;
    assert(false);
  }

  std::vector<VkSpecializationMapEntry> entries;
  const bool hasSpec = !info.specInfo.enteries.empty();
  VkSpecializationInfo specInfo;

  if (hasSpec) {
    entries.reserve(info.specInfo.enteries.size());
    uint32_t offset = 0;
    for (auto &spec : info.specInfo.enteries) {
      entries.emplace_back(VkSpecializationMapEntry{
          .constantID = spec.constantID,
          .offset = offset,
          .size = spec.size,
      });
      offset += spec.size;
    }

    specInfo = {
        .mapEntryCount = static_cast<uint32_t>(entries.size()),
        .pMapEntries = entries.data(),
        .dataSize = info.specInfo.dataSize,
        .pData = info.specInfo.data,
    };
  }

  VkPipelineLayout layout = ctx->createPipelineLayout(info.pushConstantSize);

  VkPipelineShaderStageCreateInfo computeStage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = info.comp->getShader(),
      .pName = "main",
      .pSpecializationInfo = hasSpec ? &specInfo : nullptr,
  };

  VkComputePipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = computeStage,
      .layout = layout,
  };

  VkPipeline pipeline;
  if (vkCreateComputePipelines(ctx->device, VK_NULL_HANDLE, 1, &pipelineInfo,
                               nullptr, &pipeline) != VK_SUCCESS)
    throw std::runtime_error("failed to create compute pipeline");

  Pipeline *pm = new Pipeline(ctx->device, pipeline, layout);
  return pm;
}

struct Buffer *Renderer::createBuffer(const struct BufferInfo &info) {
  VkBufferUsageFlags usageFlags =
      info.storage == BufferStorage::StorageType_Device
          ? VK_BUFFER_USAGE_TRANSFER_DST_BIT
          : VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  if (info.usage & BufferUsage::VertexBuffer)
    usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (info.usage & BufferUsage::IndexBuffer)
    usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (info.usage & BufferUsage::StorageBuffer)
    usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                  VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  if (info.usage & BufferUsage::UniformBuffer)
    usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                  VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

#ifdef MAI_USE_VMA
  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo allocInfo;

  BufferDesc bufferInfo = {
      .size = info.size,
      .usage = usageFlags,
      .memoryUsage = VMA_MEMORY_USAGE_AUTO,
  };

  if (info.storage == BufferStorage::StorageType_Device)
    bufferInfo.allocflags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  else
    bufferInfo.allocflags |=
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT;

  ctx->createBuffer(bufferInfo, buffer, allocation, allocInfo);

  if (info.data) {
    ctx->updateBuffer(usageFlags, buffer, allocation, info.size, info.data);
  }

  Buffer *bufferModule =
      new Buffer(ctx->allocator, buffer, allocation, allocInfo, usageFlags);
  return bufferModule;

#else
  VkBuffer buffer;
  VkDeviceMemory bufferMemory;
  ctx->createBuffer(info.size, usageFlags,
                    info.storage == BufferStorage::StorageType_Device
                        ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                        : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    buffer, bufferMemory);

  if (info.data) {
    ctx->updateBuffer(usageFlags, buffer, bufferMemory, info.size, info.data);
  }
  Buffer *bufferModule =
      new Buffer(ctx->device, buffer, bufferMemory, usageFlags);
  return bufferModule;

#endif
}

struct Texture *Renderer::createImage(const struct TextureInfo &info) {
  VkFormat format_ = getFormat(info.format);

  VkDeviceSize imageSize = info.dimensions.width * info.dimensions.height * 4;
  uint32_t layerCount = 1;

  ImageDesc imageInfo = {
      .type = VK_IMAGE_TYPE_2D,
      .extent =
          {
              .width = info.dimensions.width,
              .height = info.dimensions.height,
              .depth = info.dimensions.depth,
          },
      .format = format_,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = info.usage != Sampled_Bit ? getImageUsage(info.usage)
                                         : getImageUsage(info.usage) |
                                               VK_IMAGE_USAGE_TRANSFER_DST_BIT,
  };

  if (info.type == MAI::TextureType_Cube) {
    imageSize *= sizeof(float) * 6;
    layerCount = 6;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.arrayLayers = 6;
  }

#ifdef MAI_USE_VMA
  VkImage image;
  VmaAllocation allocation;

  ctx->createImage(imageInfo, image, allocation);

  if (info.usage == MAI::Sampled_Bit) {
    if (!info.data)
      throw std::runtime_error("texture have no data to it");

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    VmaAllocationInfo stagingAllocInfo;

    ctx->createBuffer(
        {
            .size = imageSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .allocflags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_AUTO,
        },
        stagingBuffer, stagingAllocation, stagingAllocInfo);
    memcpy(stagingAllocInfo.pMappedData, info.data, imageSize);

    ctx->transitionImageLayout(image, format_, VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               layerCount);
    ctx->copyBuffeToImage(stagingBuffer, image,
                          static_cast<uint32_t>(info.dimensions.width),
                          static_cast<uint32_t>(info.dimensions.height),
                          info.type == MAI::TextureType_Cube);
    ctx->transitionImageLayout(
        image, format_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layerCount);
    vmaDestroyBuffer(ctx->allocator, stagingBuffer, stagingAllocation);
  }
#else
  VkImage image;
  VkDeviceMemory imageMemory;
  ctx->createImage(imageInfo, image, imageMemory);

  if (info.usage == MAI::Sampled_Bit) {
    if (!info.data)
      throw std::runtime_error("texture have no data to it");

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    ctx->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      stagingBuffer, stagingMemory);
    void *data;
    vkMapMemory(ctx->device, stagingMemory, 0, imageSize, 0, &data);
    memcpy(data, info.data, static_cast<size_t>(imageSize));
    vkUnmapMemory(ctx->device, stagingMemory);
    ctx->transitionImageLayout(image, format_, VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               layerCount);

    ctx->copyBuffeToImage(stagingBuffer, image,
                          static_cast<uint32_t>(info.dimensions.width),
                          static_cast<uint32_t>(info.dimensions.height),
                          info.type == MAI::TextureType_Cube);

    ctx->transitionImageLayout(
        image, format_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layerCount);

    vkDestroyBuffer(ctx->device, stagingBuffer, nullptr);
    vkFreeMemory(ctx->device, stagingMemory, nullptr);
  }

#endif

  VkImageView imageView = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;

  if (info.type == MAI::TextureType_2D) {
    VkImageAspectFlags aspect = info.usage == TextureUsage::Attachment_Bit
                                    ? VK_IMAGE_ASPECT_DEPTH_BIT
                                    : VK_IMAGE_ASPECT_COLOR_BIT;

    ctx->createImageView(
        {
            .format = getFormat(info.format),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .aspect = aspect,
        },
        image, imageView);

    ctx->createSampler(
        {
            .minFilter = MAI::Linear,
            .magFilter = MAI::Linear,
            .mipMap = SamplerMipmap::Mode_Linear,
            .wrapU = SamplerWrap::Repeat,
            .wrapV = SamplerWrap::Repeat,
            .wrapW = SamplerWrap::Repeat,
        },
        sampler);
  } else if (info.type == MAI::TextureType_Cube) {
    ctx->createImageView(
        {
            .format = format_,
            .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 6,
        },
        image, imageView);
    ctx->createSampler(
        {
            .minFilter = MAI::Linear,
            .magFilter = MAI::Linear,
            .mipMap = SamplerMipmap::Mode_Linear,
            .wrapU = SamplerWrap::Clamp_to_Edge,
            .wrapV = SamplerWrap::Clamp_to_Edge,
            .wrapW = SamplerWrap::Clamp_to_Edge,
        },
        sampler);
  }

#ifdef MAI_USE_VMA
  Texture *texture = new Texture(ctx->device, ctx->allocator, image, allocation,
                                 imageView, sampler, format_);
#else
  Texture *texture =
      new Texture(ctx->device, image, imageMemory, imageView, sampler, format_);
#endif

  if (info.usage == MAI::Attachment_Bit)
    return texture;

  if (info.type == MAI::TextureType_2D) {
    lastTextureCount++;
    assert(lastTextureCount < MAX_TEXTURES);
    texture->setTextureIndex(lastTextureCount);

    ctx->updateDescriptorImageWrite(texture->getImageView(),
                                    texture->getSampler(), lastTextureCount);
  } else if (info.type == MAI::TextureType_Cube) {
    lastCubemapCount++;

    assert(lastCubemapCount < MAX_TEXTURES);
    texture->setTextureIndex(lastCubemapCount);

    ctx->updateDescriptorImageWrite(
        texture->getImageView(), texture->getSampler(), lastCubemapCount, true);
  }

  return texture;
}

CommandBuffer::CommandBuffer(VulkanContext *ctx) : ctx(ctx) {}

void CommandBuffer::cmdBeginRendering(const struct BeginInfo &info) {

  uint32_t frameIndex = ctx->frameIndex;
  VkImage swapChainImage = ctx->swapChainImages[ctx->imageIndex];

  ctx->transition_image_layout(
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0,
      VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, swapChainImage);

  if (info.texture != nullptr) {
    ctx->transition_image_layout(
        VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        info.texture->getImage());
  }

  VkClearValue clearColor = {
      {info.clearColor[0], info.clearColor[1], info.clearColor[2],
       info.clearColor[3]},
  };
  VkClearValue clearDepth = {{1.0f, 0.0f}};

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(ctx->swapChainExtent.width),
      .height = static_cast<float>(ctx->swapChainExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = ctx->swapChainExtent,
  };
  VkRenderingAttachmentInfo depthAttachmentInfo;

  if (info.texture != nullptr)
    depthAttachmentInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = info.texture->getImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = clearDepth,
    };

  VkRenderingAttachmentInfo attachmentInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = ctx->swapChainImageViews[ctx->imageIndex],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_NONE,
      .clearValue = clearColor,
  };

  VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea =
          {
              .offset = {0, 0},
              .extent = ctx->swapChainExtent,
          },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo,
  };
  if (info.texture != nullptr)
    renderingInfo.pDepthAttachment = &depthAttachmentInfo;

  VkCommandBuffer &commandBuffer = ctx->commandBuffers[frameIndex];

  vkCmdBeginRendering(commandBuffer, &renderingInfo);
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  cmdBindDepthState({
      .depthWriteEnable = false,
      .compareOp = CompareOp::Always,
  });
}

void CommandBuffer::cmdEndRendering() {

  vkCmdEndRendering(ctx->commandBuffers[ctx->frameIndex]);

  ctx->transition_image_layout(
      VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      {}, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
      ctx->swapChainImages[ctx->imageIndex]);
  vkEndCommandBuffer(ctx->commandBuffers[ctx->frameIndex]);

  lastBindPipline = nullptr;
}

void CommandBuffer::bindPipeline(Pipeline *pipeline) {
  assert(pipeline->getPipeline() != VK_NULL_HANDLE);
  lastBindPipline = pipeline;
  if (lastBindPipline != nullptr) {
    vkCmdBindPipeline(ctx->commandBuffers[ctx->frameIndex],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      lastBindPipline->getPipeline());
    vkCmdBindDescriptorSets(ctx->commandBuffers[ctx->frameIndex],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            lastBindPipline->getPipelineLayout(), 0, 1,
                            &ctx->descriptorSets[ctx->frameIndex], 0, nullptr);
  }
}

void CommandBuffer::bindComputePipeline(Pipeline *pipeline) {
  assert(pipeline->getPipeline() != VK_NULL_HANDLE);
  lastBindComputePipeline = pipeline;
  if (lastBindComputePipeline != nullptr) {
    vkCmdBindPipeline(ctx->commandBuffers[ctx->frameIndex],
                      VK_PIPELINE_BIND_POINT_COMPUTE,
                      lastBindComputePipeline->getPipeline());
    vkCmdBindDescriptorSets(ctx->commandBuffers[ctx->frameIndex],
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            lastBindComputePipeline->getPipelineLayout(), 0, 1,
                            &ctx->descriptorSets[ctx->frameIndex], 0, nullptr);
  }
}

void CommandBuffer::bindVertexBuffer(uint32_t firstBinding, Buffer *buffer,
                                     uint32_t offset) {
  VkBuffer vertexBuffer[] = {buffer->getBuffer()};
  VkDeviceSize offsets[] = {offset};
  vkCmdBindVertexBuffers(ctx->commandBuffers[ctx->frameIndex], firstBinding, 1,
                         vertexBuffer, offsets);
}

void CommandBuffer::bindIndexBuffer(Buffer *buffer, VkDeviceSize offset,
                                    IndexType indexType) {
  vkCmdBindIndexBuffer(ctx->commandBuffers[ctx->frameIndex],
                       buffer->getBuffer(), offset, getIndexType(indexType));
}

void CommandBuffer::cmdDraw(uint32_t vertexCount, uint32_t instanceCount,
                            uint32_t firstVertex, uint32_t firstInstance) {
  assert(lastBindPipline);
  vkCmdDraw(ctx->commandBuffers[ctx->frameIndex], vertexCount, instanceCount,
            firstVertex, firstInstance);
}

void CommandBuffer::cmdDrawIndex(uint32_t indexCount, uint32_t instanceCount,
                                 uint32_t firstIndex, int32_t vertexOffset,
                                 uint32_t firstInstance) {
  assert(lastBindPipline);
  vkCmdDrawIndexed(ctx->commandBuffers[ctx->frameIndex], indexCount,
                   instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::cmdBindDepthState(const struct DepthState &depthInfo) {
  VkCommandBuffer &commandBuffer = ctx->commandBuffers[ctx->frameIndex];
  vkCmdSetDepthWriteEnable(commandBuffer, depthInfo.depthWriteEnable);
  vkCmdSetDepthTestEnable(
      commandBuffer,
      (depthInfo.compareOp != CompareOp::Always && depthInfo.depthWriteEnable)
          ? VK_TRUE
          : VK_FALSE);
}

void CommandBuffer::cmdPushConstant(const void *push, uint32_t size) {
  if (lastBindPipline) {
    vkCmdPushConstants(ctx->commandBuffers[ctx->frameIndex],
                       lastBindPipline->getPipelineLayout(),
                       VK_SHADER_STAGE_ALL, 0, size, push);
  } else if (lastBindComputePipeline) {
    vkCmdPushConstants(ctx->commandBuffers[ctx->frameIndex],
                       lastBindComputePipeline->getPipelineLayout(),
                       VK_SHADER_STAGE_ALL, 0, size, push);
  } else {
    std::cerr << "pipeline wasn't bound" << std::endl;
    assert(false);
  }
}

void CommandBuffer::cmdDispatchThreadGroups(
    const struct DispatchThreadInfo &info) {
  vkCmdDispatch(ctx->commandBuffers[ctx->frameIndex], info.width, info.height,
                info.depth);
}

void CommandBuffer::update(struct Buffer *buffer, const void *data,
                           size_t size) {
  VkBufferUsageFlags usage = buffer->getBufferUsage();
  VkCommandBuffer &commandBuffer = ctx->commandBuffers[ctx->frameIndex];

  VkResult result;
  VkAccessFlags2 bufAccessMask;
  if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    bufAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
  if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    bufAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
  if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    bufAccessMask = VK_ACCESS_INDEX_READ_BIT;
  if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
    bufAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
  if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
    bufAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;

#ifdef MAI_USE_VMA
    if (vmaCopyMemoryToAllocation(ctx->allocator, data, buffer->getAllocation(),
                                  0, size) != VK_SUCCESS)
      throw std::runtime_error("failed to update host visible buffer");
#else
    void *mappedData;
    vkMapMemory(ctx->device, buffer->getBufferMem(), 0, size, 0, &mappedData);
    memcpy(mappedData, data, size);
    vkUnmapMemory(ctx->device, buffer->getBufferMem());
#endif

    VkBufferMemoryBarrier bufMemBarrier = {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    bufMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    bufMemBarrier.dstAccessMask = bufAccessMask;
    bufMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufMemBarrier.buffer = buffer->getBuffer();
    bufMemBarrier.offset = 0;
    bufMemBarrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT,
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1,
                         &bufMemBarrier, 0, nullptr);

  } else if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
#ifdef MAI_USE_VMA
    BufferDesc bufferInfo{
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .allocflags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .memoryUsage = VMA_MEMORY_USAGE_AUTO,
    };

    VkBuffer stagingBuf;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingAllocInfo;
    ctx->createBuffer(bufferInfo, stagingBuf, stagingAlloc, stagingAllocInfo);

    vmaCopyMemoryToAllocation(ctx->allocator, data, stagingAlloc, 0, size);

    ctx->copyBuffer(stagingBuf, buffer->getBuffer(), size);

    vmaDestroyBuffer(ctx->allocator, stagingBuf, stagingAlloc);

#else
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    ctx->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      stagingBuffer, stagingMemory);

    void *mappedData;
    vkMapMemory(ctx->device, stagingMemory, 0, size, 0, &mappedData);
    memcpy(mappedData, data, size);
    vkUnmapMemory(ctx->device, stagingMemory);

    ctx->copyBuffer(stagingBuffer, buffer->getBuffer(), size);

    vkDestroyBuffer(ctx->device, stagingBuffer, nullptr);
    vkFreeMemory(ctx->device, stagingMemory, nullptr);
#endif

    VkBufferMemoryBarrier bufMemBarrier2 = {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    bufMemBarrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bufMemBarrier2.dstAccessMask = bufAccessMask;
    bufMemBarrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufMemBarrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufMemBarrier2.buffer = buffer->getBuffer();
    bufMemBarrier2.offset = 0;
    bufMemBarrier2.size = VK_WHOLE_SIZE;

    // Make sure copying from staging buffer to the actual buffer has finished
    // by inserting a buffer memory barrier.
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1,
                         &bufMemBarrier2, 0, nullptr);
  }
}

void Renderer::waitDeviceIdle() { vkDeviceWaitIdle(ctx->device); }

void Renderer::submit() {
  uint32_t frameIndex = ctx->frameIndex;

  VkSemaphore waitSemaphore[] = {ctx->imageAvailableSemaphore[frameIndex]};

  VkSemaphore signalSemaphore[] = {ctx->renderFinishSemaphore[ctx->imageIndex]};
  VkCommandBuffer &commandBuffer = ctx->commandBuffers[frameIndex];

  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  };

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = waitSemaphore,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signalSemaphore,
  };

  if (vkQueueSubmit(ctx->graphicsQueue, 1, &submitInfo,
                    ctx->drawFences[frameIndex]) != VK_SUCCESS)
    throw std::runtime_error("faile to submit to the queue");

  VkSwapchainKHR swapChains[] = {ctx->swapChain};

  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signalSemaphore,
      .swapchainCount = 1,
      .pSwapchains = swapChains,
      .pImageIndices = &ctx->imageIndex,
  };
  bool framedResized = false;
  VkResult result = vkQueuePresentKHR(ctx->presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      framedResized) {
    framedResized = false;
    ctx->recreateSwapChain();
  } else if (result != VK_SUCCESS)
    throw std::runtime_error("failed to present swap chain image");

  ctx->frameIndex = (ctx->frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

uint64_t Renderer::gpuAddress(struct Buffer *buffer) {
  VkBufferDeviceAddressInfo addrInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .buffer = buffer->getBuffer(),
  };
  VkDeviceAddress address = vkGetBufferDeviceAddress(ctx->device, &addrInfo);
  return address;
}

Renderer::~Renderer() { delete ctx; }

GLFWwindow *initWindow(const WindowInfo &info) {
  if (!glfwInit())
    throw std::runtime_error("failed to init glfw");

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  if (!info.allowResize)
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow *window = glfwCreateWindow(
      info.width, info.height, info.appName,
      info.isFullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

  glfwSetKeyCallback(window,
                     [](auto *window, int keys, int, int action, int mods) {
                       if (keys == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                         glfwSetWindowShouldClose(window, GLFW_TRUE);
                     });

  return window;
}

Renderer *initVulkanWithSwapChain(GLFWwindow *window, const char *appName) {
  assert(window);
  assert(appName != nullptr);
  VulkanContext *ctx = new VulkanContext(window, appName);
  Renderer *ren = new Renderer(ctx);

  return ren;
}

VulkanContext::VulkanContext(GLFWwindow *window, const char *name)
    : window(window), appName(name) {
  createInstance();
  setupDebugger();
  createSurfaceKHR();
  pickPhysicalDevice();
  createLogicalDevice();
#ifdef MAI_USE_VMA
  createVmaAllocation();
#endif

  createSwapChain();
  createSwapChainImageViews();

  createSyncObj();

  createCommandPool();
  createCommandBuffer();
  createDescriptorPool();
  createDescriptorSetLayout();
  createDescriptorSets();
}

void VulkanContext::createInstance() {
  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = appName,
      .applicationVersion = VK_API_VERSION_1_0,
      .pEngineName = appName,
      .engineVersion = VK_API_VERSION_1_0,
      .apiVersion = VK_API_VERSION_1_3,
  };
  if (enableValidation && !checkValidation())
    throw std::runtime_error("validation layer requested but not available");

  auto extensions = getRequiredExtensions();

  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),

  };

  VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
  if (enableValidation) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    populateDebugMessenger(debugInfo);
    createInfo.pNext = &debugInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    throw std::runtime_error("failed to create instance");
}

void VulkanContext::setupDebugger() {
  if (!enableValidation)
    return;
  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  populateDebugMessenger(createInfo);
  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                   &debugMessenger) != VK_SUCCESS)
    throw std::runtime_error("failed to create debug utils messenger");
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
  QueueFamilyIndices indices;
  uint32_t queueFamiliesCount;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount,
                                           nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilyProperties(
      queueFamiliesCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount,
                                           queueFamilyProperties.data());
  for (uint32_t i = 0; i < queueFamiliesCount; i++) {
    if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
        (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
      indices.graphcisFamily = i;

    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);

    if (supported)
      indices.presentFamily = i;

    if (indices.isComplete())
      break;
  }

  return indices;
}

void VulkanContext::createSurfaceKHR() {
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create window surface");
}

bool isDeviceSuitable(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       extensions.data());

  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);

  bool isSuitable = properties.apiVersion >= VK_API_VERSION_1_3;

  std::set<const char *> requiredExtensions(deviceExtensions.begin(),
                                            deviceExtensions.end());

  for (uint32_t i = 0; i < extensionCount; i++)
    for (const char *extension : deviceExtensions)
      if (strcmp(extension, extensions[i].extensionName) == 0)
        requiredExtensions.erase(extension);

  isSuitable = requiredExtensions.empty();

  return isSuitable;
}

void VulkanContext::pickPhysicalDevice() {
  uint32_t deviceCount;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0)
    throw std::runtime_error("failed to find any device with vulkan support");

  std::vector<VkPhysicalDevice> devices;
  devices.resize(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (uint32_t i = 0; i < deviceCount; i++)
    if (isDeviceSuitable(devices[i])) {
      physicalDevice = devices[i];
      break;
    }

  if (physicalDevice == nullptr)
    throw std::runtime_error("failed to find suitable GPU");
}

void VulkanContext::createLogicalDevice() {
  indices = findQueueFamilies(physicalDevice, surface);

  std::set<uint32_t> uniqueQueueFamilies = {indices.graphcisFamily.value(),
                                            indices.presentFamily.value()};
  std::vector<VkDeviceQueueCreateInfo> deviceQueueInfos;
  float queuePriority = 0.5f;

  for (const auto &uniqueQueue : uniqueQueueFamilies)
    deviceQueueInfos.emplace_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = uniqueQueue,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    });

  VkPhysicalDeviceBufferDeviceAddressFeatures bda{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
      .bufferDeviceAddress = VK_TRUE,
  };

  VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
      .pNext = &bda,
      .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
      .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
      .descriptorBindingPartiallyBound = VK_TRUE,
      .descriptorBindingVariableDescriptorCount = VK_TRUE,
      .runtimeDescriptorArray = VK_TRUE,
  };

  VkPhysicalDeviceFeatures deviceFeatures{
      .geometryShader = VK_TRUE,
      .tessellationShader = VK_TRUE,
      .depthBiasClamp = VK_TRUE,
      .fillModeNonSolid = VK_TRUE,
      .samplerAnisotropy = VK_TRUE,
  };

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicStateFeatues = {
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .pNext = &indexingFeatures,
      .extendedDynamicState = true,
  };

  VkPhysicalDeviceVulkan13Features vulkan13Features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
      .pNext = &dynamicStateFeatues,
      .synchronization2 = true,
      .dynamicRendering = true,
  };

  VkPhysicalDeviceFeatures2 deviceFeatures2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &vulkan13Features,
      .features = deviceFeatures,
  };

  VkDeviceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &deviceFeatures2,
      .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueInfos.size()),
      .pQueueCreateInfos = deviceQueueInfos.data(),
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data(),
  };

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create logical device");

  vkGetDeviceQueue(device, indices.graphcisFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

#ifdef MAI_USE_VMA
void VulkanContext::createVmaAllocation() {
  std::cout << "VMA is enabled" << std::endl;
  VmaAllocatorCreateInfo createInfo{
      .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
               VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = physicalDevice,
      .device = device,
      .instance = instance,
      .vulkanApiVersion = VK_API_VERSION_1_3,
  };
  if (vmaCreateAllocator(&createInfo, &allocator) != VK_SUCCESS)
    throw std::runtime_error("failed to create vma allocator");
}
#endif

void VulkanContext::createSwapChain() {
  uint32_t formatCount;
  SwapChainSupportDetails swapChainDetails =
      querrySwapChainSupport(physicalDevice, surface);
  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapChainFormat(swapChainDetails.surfaceFormats);
  VkPresentModeKHR presentMode =
      chooseSwapChainPresentMode(swapChainDetails.presentModes);
  VkExtent2D extents =
      chooseSwapChainExtent(swapChainDetails.capabilities, window);

  uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;

  if (swapChainDetails.capabilities.maxImageCount > 0 &&
      imageCount < swapChainDetails.capabilities.maxImageCount)
    imageCount = swapChainDetails.capabilities.maxImageCount;

  VkSwapchainCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extents,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = swapChainDetails.capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = presentMode,
      .clipped = true,
      .oldSwapchain = nullptr,
  };

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create swap chain");

  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
                          swapChainImages.data());
  swapChainFormat = surfaceFormat.format;
  swapChainColoSpace = surfaceFormat.colorSpace;
  swapChainExtent = extents;
}

void VulkanContext::createSwapChainImageViews() {
  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = swapChainImages[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapChainFormat,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    if (vkCreateImageView(device, &createInfo, nullptr,
                          &swapChainImageViews[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to create swap chain image views");
  }
}

void VulkanContext::cleanupSwapChain() {
  for (size_t i = 0; i < swapChainImages.size(); i++)
    vkDestroyImageView(device, swapChainImageViews[i], nullptr);

  vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanContext::recreateSwapChain() {
  vkDeviceWaitIdle(device);
  cleanupSwapChain();

  createSwapChain();
  createSwapChainImageViews();
}

void VulkanContext::createSyncObj() {
  imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishSemaphore.resize(swapChainImages.size());
  drawFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  for (size_t i = 0; i < swapChainImages.size(); i++)
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &renderFinishSemaphore[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to create renderSemaphore");

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &imageAvailableSemaphore[i]) ||
        vkCreateFence(device, &fenceInfo, nullptr, &drawFences[i]) !=
            VK_SUCCESS)
      throw std::runtime_error("failed to create fence");
}

void VulkanContext::createCommandPool() {
  VkCommandPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = indices.graphcisFamily.value(),
  };

  if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to get command pool");
}

void VulkanContext::createCommandBuffer() {
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
  };

  if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to allocate command buffer");
}

void VulkanContext::createDescriptorPool() {
  VkDescriptorPoolSize poolSize[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT},
      {VK_DESCRIPTOR_TYPE_SAMPLER, MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT},
  };
  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 3,
      .pPoolSizes = poolSize,
  };
  if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create descritptor fool");
}

void VulkanContext::createDescriptorSetLayout() {
  std::vector<VkDescriptorBindingFlags> bindingFlags = {
      // binding 0
      // (2D texture)
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
          VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
      // binding 1
      // (sampler)
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
          VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
          // binding 2
          // (cubemap)
          VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
          VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
      VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
  };
  std::vector<VkDescriptorSetLayoutBinding> uboLayout({
      // 2d textures
      {
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
          .descriptorCount = MAX_TEXTURES,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      {
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
          .descriptorCount = MAX_TEXTURES,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
      // cubemap
      {
          .binding = 2,
          .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
          .descriptorCount = MAX_TEXTURES,
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      },
  });

  VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCreateInfo = {
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
      .pBindingFlags = bindingFlags.data(),
  };
  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &flagsCreateInfo,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
      .bindingCount = static_cast<uint32_t>(uboLayout.size()),
      .pBindings = uboLayout.data(),
  };

  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS)
    throw std::runtime_error("failed to create descriptor set layouts");
}

void VulkanContext::createDescriptorSets() {
  descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             descriptorSetLayout);
  uint32_t counts[] = {MAX_TEXTURES, MAX_TEXTURES};
  VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
      .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
      .pDescriptorCounts = counts,
  };

  VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = &countInfo,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
      .pSetLayouts = layouts.data(),
  };

  if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to allocate descriptor set");
}

uint32_t VulkanContext::findMemoryType(uint32_t typeFilter,
                                       VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;

  throw std::runtime_error("failed to find suitable memory type!");
}

VkCommandBuffer VulkanContext::beginSingleCommandBuffer() {
  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void VulkanContext::endSingleCommandBuffer(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);
  VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
  };

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);
  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanContext::transitionImageLayout(VkImage image, VkFormat format,
                                          VkImageLayout oldLayout,
                                          VkImageLayout newLayout,
                                          uint32_t layerCount) {
  VkCommandBuffer commandBuffer = beginSingleCommandBuffer();
  VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = layerCount,
          },

  };

  VkPipelineStageFlags sourcesStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourcesStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourcesStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else
    throw std::invalid_argument("unsupported layout transition!");

  vkCmdPipelineBarrier(commandBuffer, sourcesStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  endSingleCommandBuffer(commandBuffer);
}

void VulkanContext::copyBuffeToImage(VkBuffer buffer, VkImage image,
                                     uint32_t width, uint32_t height,
                                     bool isCube) {

  VkCommandBuffer commandBuffer = beginSingleCommandBuffer();

  if (!isCube) {
    VkBufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,

        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1},
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  } else {
    std::array<VkBufferImageCopy, 6> regions{};
    VkDeviceSize faceSize = width * height * 4 * sizeof(float);
    for (uint32_t face = 0; face < 6; face++) {
      regions[face].bufferOffset = face * faceSize;
      regions[face].bufferRowLength = 0;
      regions[face].bufferImageHeight = 0;

      regions[face].imageSubresource = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .mipLevel = 0,
          .baseArrayLayer = face,
          .layerCount = 1,
      };
      regions[face].imageOffset = {0, 0, 0};
      regions[face].imageExtent = {width, height, 1};
    }

    vkCmdCopyBufferToImage(
        commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(regions.size()), regions.data());
  }

  endSingleCommandBuffer(commandBuffer);
}

void VulkanContext::updateDescriptorImageWrite(VkImageView imageView,
                                               VkSampler sampler,
                                               uint32_t imageIndex,
                                               bool isCubemap) {
  VkDescriptorImageInfo imageInfo{
      .imageView = imageView,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  VkDescriptorImageInfo samplerInfo{
      .sampler = sampler,
  };

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = imageIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &imageInfo,
    };
    if (isCubemap)
      descriptorWrites[0].dstBinding = 2;

    descriptorWrites[1] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 1,
        .dstArrayElement = imageIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &samplerInfo,
    };

    vkUpdateDescriptorSets(device, descriptorWrites.size(),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void VulkanContext::transition_image_layout(
    VkImageAspectFlags imageAspect, VkImageLayout oldLayout,
    VkImageLayout newLayout, VkAccessFlagBits2 srcAccessMask,
    VkAccessFlagBits2 dstAccessMask, VkPipelineStageFlags2 srcStageMask,
    VkPipelineStageFlags2 dstStageMask, VkImage &image) {

  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = srcStageMask,
      .srcAccessMask = srcAccessMask,
      .dstStageMask = dstStageMask,
      .dstAccessMask = dstAccessMask,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = imageAspect,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  VkDependencyInfo dependencyInfo = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };

  vkCmdPipelineBarrier2(commandBuffers[frameIndex], &dependencyInfo);
}

VkShaderModule VulkanContext::createShaderModule(uint32_t codeSize,
                                                 const void *code) {
  VkShaderModuleCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = codeSize,
      .pCode = reinterpret_cast<const uint32_t *>(code),
  };

  VkShaderModule sm;

  if (vkCreateShaderModule(device, &createInfo, nullptr, &sm))
    throw std::runtime_error("failed to create shader module");
  return sm;
}
VkPipelineLayout
VulkanContext::createPipelineLayout(uint32_t pushConstantSize) {
  VkPushConstantRange range = {
      .stageFlags = VK_SHADER_STAGE_ALL,
      .offset = 0,
      .size = pushConstantSize,
  };

  VkPipelineLayoutCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptorSetLayout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &range,
  };

  VkPipelineLayout pipelineLayout;
  if (vkCreatePipelineLayout(device, &createInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout");

  return pipelineLayout;
}

VkPipeline
VulkanContext::createGraphicePipeline(VkGraphicsPipelineCreateInfo &info) {
  VkPipeline pipeline;
  if (vkCreateGraphicsPipelines(device, nullptr, 1, &info, nullptr,
                                &pipeline) != VK_SUCCESS)
    throw std::runtime_error("failed to create graphics pipeline");

  return pipeline;
}

#ifdef MAI_USE_VMA
void VulkanContext::createBuffer(const struct BufferDesc &info,
                                 VkBuffer &buffer, VmaAllocation &allocation,
                                 VmaAllocationInfo &allocInfo) {
  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = info.size,
      .usage = info.usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  VmaAllocationCreateInfo allocCreateInfo = {
      .flags = info.allocflags,
      .usage = info.memoryUsage,
      .priority = 1.0f,
  };

  if (vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer,
                      &allocation, &allocInfo) != VK_SUCCESS)
    throw std::runtime_error("failed to create buffer usage");
}
void VulkanContext::updateBuffer(VkBufferUsageFlags usage, VkBuffer &buffer,
                                 VmaAllocation &alloc, size_t size,
                                 const void *data) {
  if (!data)
    throw std::runtime_error("buffer data is empty");

  if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
    if (vmaCopyMemoryToAllocation(allocator, data, alloc, 0, size) !=
        VK_SUCCESS)
      throw std::runtime_error("failed to update host visible buffer");
  } else if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
    BufferDesc bufferInfo{
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .allocflags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .memoryUsage = VMA_MEMORY_USAGE_AUTO,
    };

    VkBuffer stagingBuf;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingAllocInfo;
    createBuffer(bufferInfo, stagingBuf, stagingAlloc, stagingAllocInfo);

    vmaCopyMemoryToAllocation(allocator, data, stagingAlloc, 0, size);

    copyBuffer(stagingBuf, buffer, size);

    vmaDestroyBuffer(allocator, stagingBuf, stagingAlloc);
  }
}

void VulkanContext::createImage(const struct ImageDesc &info, VkImage &image,
                                VmaAllocation &alloc) {
  VkImageCreateInfo imageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .flags = info.flags,
      .imageType = info.type,
      .format = info.format,
      .extent = info.extent,
      .mipLevels = info.mipLevel,
      .arrayLayers = info.arrayLayers,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = info.tiling,
      .usage = info.usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo allocaCreateInfo{
      .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO,
      .priority = 1.0f,
  };

  if (vmaCreateImage(allocator, &imageCreateInfo, &allocaCreateInfo, &image,
                     &alloc, nullptr) != VK_SUCCESS)
    throw std::runtime_error("failed to create image");
}
#else
void VulkanContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer &buffer,
                                 VkDeviceMemory &bufferMemory) {
  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    throw std::runtime_error("failed to create buffer module");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties),
  };

  if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
    VkMemoryAllocateFlagsInfo allocFlags = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
    };
    allocInfo.pNext = &allocFlags;
  } else
    allocInfo.pNext = nullptr;

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to allocate buffer memory");

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanContext::updateBuffer(VkBufferUsageFlags usage, VkBuffer &buffer,
                                 VkDeviceMemory &deviceMemory, size_t size,
                                 const void *bufferData) {

  if (!bufferData)
    throw std::runtime_error("buffer data is empty");

  if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
    void *data;
    vkMapMemory(device, deviceMemory, 0, size, 0, &data);
    memcpy(data, bufferData, static_cast<size_t>(size));
    vkUnmapMemory(device, deviceMemory);
  } else if (VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingMemory);
    void *data;
    vkMapMemory(device, stagingMemory, 0, size, 0, &data);
    memcpy(data, bufferData, static_cast<size_t>(size));
    vkUnmapMemory(device, stagingMemory);

    copyBuffer(stagingBuffer, buffer, size);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);
  }
}

void VulkanContext::createImage(const struct ImageDesc &info, VkImage &image,
                                VkDeviceMemory &imageMemory) {
  VkImageCreateInfo imageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .flags = info.flags,
      .imageType = info.type,
      .format = info.format,
      .extent = info.extent,
      .mipLevels = info.mipLevel,
      .arrayLayers = info.arrayLayers,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = info.tiling,
      .usage = info.usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
    throw std::runtime_error("failed to create texture image");

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
  };

  if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate image memory");

  vkBindImageMemory(device, image, imageMemory, 0);
}
#endif

void VulkanContext::createImageView(const struct ImageViewDesc &info,
                                    VkImage &image, VkImageView &view) {
  VkImageViewCreateInfo viewInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = info.viewType,
      .format = info.format,
      .subresourceRange =
          {
              .aspectMask = info.aspect,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = info.layerCount,
          },
  };

  if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS)
    throw std::runtime_error("failed to create image view");
}

void VulkanContext::createSampler(const struct SamplerDesc &samplerInfo,
                                  VkSampler &sampler) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);

  VkSamplerCreateInfo samplerCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = getSamplerFilter(samplerInfo.magFilter),
      .minFilter = getSamplerFilter(samplerInfo.minFilter),
      .mipmapMode = getSamplerMipmapMode(samplerInfo.mipMap),
      .addressModeU = getSamplerWrap(samplerInfo.wrapU),
      .addressModeV = getSamplerWrap(samplerInfo.wrapV),
      .addressModeW = getSamplerWrap(samplerInfo.wrapW),
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
      .compareOp = getCompareOp(samplerInfo.depthCompareOp),
  };
  samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerCreateInfo.compareEnable = samplerInfo.depthCompareEnabled;
  samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

  if (vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler) !=
      VK_SUCCESS)
    throw std::runtime_error("failed to create texture sampler");
}

void VulkanContext::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                               VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginSingleCommandBuffer();

  VkBufferCopy copyRegion{
      .size = size,
  };
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleCommandBuffer(commandBuffer);
}

void VulkanContext::acquireSwapChainIndex() {
  if (vkWaitForFences(device, 1, &drawFences[frameIndex], VK_TRUE,
                      UINT64_MAX) != VK_SUCCESS)
    throw std::runtime_error("failed to wait for draw fence");

  vkResetFences(device, 1, &drawFences[frameIndex]);

  VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                                          imageAvailableSemaphore[frameIndex],
                                          nullptr, &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    recreateSwapChain();
}

VulkanContext::~VulkanContext() {

  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);

  vkDestroyCommandPool(device, commandPool, nullptr);

  for (size_t i = 0; i != MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
    vkDestroyFence(device, drawFences[i], nullptr);
  }

  for (size_t i = 0; i != swapChainImages.size(); i++)
    vkDestroySemaphore(device, renderFinishSemaphore[i], nullptr);

  cleanupSwapChain();
#ifdef MAI_USE_VMA
  vmaDestroyAllocator(allocator);
#endif

  vkDestroyDevice(device, nullptr);
  vkDestroySurfaceKHR(instance, surface, nullptr);
  DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

  vkDestroyInstance(instance, nullptr);
}

bool checkValidation() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;
    for (const auto &layerProperties : availableLayers)
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    if (!layerFound)
      return false;
  }
  return true;
}

std::vector<const char *> getRequiredExtensions() {
  uint32_t glfwExtensionsCount = 0;
  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionsCount);
  if (enableValidation)
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return extensions;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT type,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallback, void *) {
  std::cerr << "validation layer: " << pCallback->pMessage << std::endl;
  return VK_FALSE;
}
VkResult
CreateDebugUtilsMessengerEXT(VkInstance instance,
                             VkDebugUtilsMessengerCreateInfoEXT *createInfo,
                             const VkAllocationCallbacks *pAllocator,
                             VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
    return func(instance, createInfo, nullptr, pDebugMessenger);
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance, messenger, nullptr);
}

void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debugCallback,
  };
}

VkSurfaceFormatKHR
chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats)
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return availableFormat;
  return availableFormats[0];
}

VkPresentModeKHR chooseSwapChainPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &presentMode : availablePresentModes)
    if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      return presentMode;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR &capabilities,
                                 GLFWwindow *window) {
  assert(window);
  if (capabilities.minImageExtent.width != std::numeric_limits<uint32_t>::max())
    return capabilities.minImageExtent;
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  VkExtent2D extent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height),
  };

  extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width);
  extent.height = std::clamp(extent.height, capabilities.minImageExtent.height,
                             capabilities.maxImageExtent.height);
  return extent;
}

SwapChainSupportDetails querrySwapChainSupport(VkPhysicalDevice device,
                                               VkSurfaceKHR surface) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
  if (formatCount != 0) {
    details.surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.surfaceFormats.data());
  }
  uint32_t presentCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount,
                                            nullptr);
  if (presentCount != 0) {
    details.presentModes.resize(presentCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount,
                                              details.presentModes.data());
  }

  return details;
}

bool endsWith(const char *s, const char *part) {
  const size_t sLength = strlen(s);
  const size_t partLength = strlen(part);
  return sLength < partLength ? false
                              : strcmp(s + sLength - partLength, part) == 0;
}

MAI::ShaderStage getShaderStageFromFile(const char *filename) {
  if (endsWith(filename, "vert") || endsWith(filename, "vspv"))
    return ShaderStage::Vert;
  else if (endsWith(filename, "frag") || endsWith(filename, "fspv"))
    return ShaderStage::Frag;
  else if (endsWith(filename, "geom") || endsWith(filename, "gspv"))
    return ShaderStage::Geom;
  else if (endsWith(filename, "tese") || endsWith(filename, "tespv"))
    return ShaderStage::Tese;
  else if (endsWith(filename, "tesc") || endsWith(filename, "tcspv"))
    return ShaderStage::Tece;
  else if (endsWith(filename, "comp") || endsWith(filename, "cspv"))
    return ShaderStage::Comp;

  assert(false);
}

std::string readShaderFile(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    std::cerr << filename << std::endl;
    assert(false);
  }
  fseek(file, 0L, SEEK_END);
  const size_t bytesinfile = ftell(file);
  fseek(file, 0L, SEEK_SET);

  char *buffer = (char *)alloca(bytesinfile + 1);
  const size_t bytesread = fread(buffer, 1, bytesinfile, file);
  fclose(file);

  buffer[bytesread] = 0;

  static constexpr unsigned char BOM[] = {0xEF, 0xBB, 0xBF};

  if (bytesread > 3) {
    if (!memcmp(buffer, BOM, 3))
      memset(buffer, ' ', 3);
  }

  std::string code(buffer);

  while (code.find("#include ") != code.npos) {
    const auto pos = code.find("#include ");
    const auto p1 = code.find('<', pos);
    const auto p2 = code.find('>', pos);
    if (p1 == code.npos || p2 == code.npos || p2 <= p1) {
      assert(false);
      return std::string();
    }
    const std::string name = code.substr(p1 + 1, p2 - p1 - 1);
    std::string dir = filename;
    size_t last_dir = dir.find_last_of("/\\");
    dir = dir.substr(0, last_dir);
    dir += "/";

    const std::string include = readShaderFile((dir + name).c_str());
    code.replace(pos, p2 - pos + 1, include.c_str());
  }

  return code;
}

std::vector<char> readShaderBinaryFile(const char *filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    std::cerr << filename << std::endl;
    assert(false);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

VkShaderStageFlags getShaderStage(MAIFlags stage) {
  VkShaderStageFlags stages = 0;
  if (stage & Vert)
    stages |= VK_SHADER_STAGE_VERTEX_BIT;
  if (stage & Frag)
    stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
  if (stage & Geom)
    stages |= VK_SHADER_STAGE_GEOMETRY_BIT;
  if (stage & Tese)
    stages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
  if (stage & Tece)
    stages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  if (stage & Comp)
    stages |= VK_SHADER_STAGE_COMPUTE_BIT;

  assert(stage != 0);

  return stages;
}

VkShaderStageFlagBits getShaderStageBits(ShaderStage stage) {
  if (stage == ShaderStage::Vert)
    return VK_SHADER_STAGE_VERTEX_BIT;
  else if (stage == ShaderStage::Frag)
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  else if (stage == ShaderStage::Geom)
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  else if (stage == ShaderStage::Tese)
    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
  else if (stage == ShaderStage::Tece)
    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  else if (stage == ShaderStage::Comp)
    return VK_SHADER_STAGE_COMPUTE_BIT;

  assert(false);
}

VkFormat getVertexFormat(VertexFormat format) {
  switch (format) {
  case Float:
    return VK_FORMAT_R32_SFLOAT;
  case Float2:
    return VK_FORMAT_R32G32_SFLOAT;
  case Float3:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case Float4:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  }
  assert(false);
}

VkVertexInputRate getVertexInputRate(InputRate rate) {
  switch (rate) {
  case Vertex:
    return VK_VERTEX_INPUT_RATE_VERTEX;
  case Instance:
    return VK_VERTEX_INPUT_RATE_INSTANCE;
  }
  assert(false);
}

VkIndexType getIndexType(IndexType type) {
  switch (type) {
  case MAI::Uint32:
    return VK_INDEX_TYPE_UINT32;
  case MAI::Uint16:
    return VK_INDEX_TYPE_UINT16;
  }
  assert(false);
}

VkCullModeFlags getCullMode(CullMode mode) {
  switch (mode) {
  case None:
    return VK_CULL_MODE_NONE;
  case Front:
    return VK_CULL_MODE_FRONT_BIT;
  case Back:
    return VK_CULL_MODE_BACK_BIT;
  case Front_Back:
    return VK_CULL_MODE_FRONT_AND_BACK;
  case Flag_Bit:
    return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
  }

  assert(false);
}

VkPolygonMode getPolygonMode(PolygonMode mode) {
  switch (mode) {
  case Fill:
    return VK_POLYGON_MODE_FILL;
  case Line:
    return VK_POLYGON_MODE_LINE;
  case Point:
    return VK_POLYGON_MODE_POINT;
  }

  assert(false);
}

VkPrimitiveTopology getPrimitiveTopology(PrimitiveTopology topology) {
  switch (topology) {
  case Line_List:
    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
  case Line_Strip:
    return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
  case Point_List:
    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
  case Triangle_List:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  case Triangle_Strip:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  case Patch_List:
    return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
  }

  assert(false);
}

VkBufferUsageFlags getBufferUsageFlags(MAIFlags usages) {
  VkBufferUsageFlags bufferUsage = 0;
  if (usages & VertexBuffer)
    bufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (usages & IndexBuffer)
    bufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (usages & UniformBuffer)
    bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if (usages & IndirectBuffer)
    bufferUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  if (usages & StorageBuffer)
    bufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

  assert(bufferUsage != 0);
  return bufferUsage;
}

VkFormat getFormat(TextureFormat format) {
  switch (format) {
  case MAI::Format_Z_F32:
    return VK_FORMAT_D32_SFLOAT;
  case MAI::Format_RGBA_S8:
    return VK_FORMAT_R8G8B8A8_SRGB;
  case MAI::Format_RGBA_F32:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  }
  assert(false);
}

VkImageUsageFlags getImageUsage(TextureUsage usage) {
  switch (usage) {
  case TextureUsage::Attachment_Bit:
    return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  case TextureUsage::Sampled_Bit:
    return VK_IMAGE_USAGE_SAMPLED_BIT;
  }
  assert(false);
}

VkFilter getSamplerFilter(SamplerFilter filter) {
  switch (filter) {
  case SamplerFilter::Linear:
    return VK_FILTER_LINEAR;
  case SamplerFilter::Nearst:
    return VK_FILTER_NEAREST;
  }
  assert(false);
}

VkSamplerMipmapMode getSamplerMipmapMode(SamplerMipmap mode) {
  switch (mode) {
  case SamplerMipmap::Mode_Linear:
    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  case SamplerMipmap::Mode_Neart:
    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
  }
  assert(false);
}

VkSamplerAddressMode getSamplerWrap(SamplerWrap wrap) {
  switch (wrap) {
  case SamplerWrap::Repeat:
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  case SamplerWrap::Clamp_to_Edge:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  case SamplerWrap::Clamp_to_Border:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  case SamplerWrap::Mirror_Clamp_to_edge:
    return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
  case SamplerWrap::Mirrored_Repeat:
    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  }
  assert(false);
}

VkCompareOp getCompareOp(CompareOp op) {
  switch (op) {
  case MAI::Never:
    return VK_COMPARE_OP_NEVER;
  case MAI::Less:
    return VK_COMPARE_OP_LESS;
  case MAI::Equal:
    return VK_COMPARE_OP_EQUAL;
  case MAI::Less_or_Equal:
    return VK_COMPARE_OP_LESS_OR_EQUAL;
  case MAI::Greater:
    return VK_COMPARE_OP_GREATER;
  case MAI::Not_Equal:
    return VK_COMPARE_OP_NOT_EQUAL;
  case MAI::Greater_or_Equal:
    return VK_COMPARE_OP_GREATER_OR_EQUAL;
  case MAI::Always:
    return VK_COMPARE_OP_ALWAYS;
  }
  assert(false);
}

VkBlendFactor getBlendFactor(BlendFactor factor) {
  switch (factor) {
  case MAI::Src_Alpha:
    return VK_BLEND_FACTOR_SRC_ALPHA;
  case MAI::Dst_Alpha:
    return VK_BLEND_FACTOR_DST_COLOR;
  case MAI::Minus_Src_Alpha:
    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  }
  assert(false);
}

#ifdef MAI_INCLUDE_GLSLANG

glslang_resource_t getGLSLangResources(const VkPhysicalDeviceLimits &limits) {
  const glslang_resource_t resource = {
      .max_lights = 32,
      .max_clip_planes = (int)limits.maxClipDistances,
      .max_texture_units = 32,
      .max_texture_coords = 32,
      .max_vertex_attribs = (int)limits.maxVertexInputAttributes,
      .max_vertex_uniform_components = (int)limits.maxUniformBufferRange / 4,
      .max_varying_floats = (int)std::min(limits.maxVertexOutputComponents,
                                          limits.maxFragmentInputComponents),
      .max_vertex_texture_image_units = 32,
      .max_combined_texture_image_units = 80,
      .max_texture_image_units = 32,
      .max_fragment_uniform_components = 4096,
      .max_draw_buffers = 32,
      .max_vertex_uniform_vectors = 128,
      .max_varying_vectors = 8,
      .max_fragment_uniform_vectors = 16,
      .max_vertex_output_vectors = (int)limits.maxVertexOutputComponents / 4,
      .max_fragment_input_vectors = (int)limits.maxFragmentInputComponents / 4,
      .min_program_texel_offset = limits.minTexelOffset,
      .max_program_texel_offset = (int)limits.maxTexelOffset,
      .max_clip_distances = (int)limits.maxClipDistances,
      .max_compute_work_group_count_x = (int)limits.maxComputeWorkGroupCount[0],
      .max_compute_work_group_count_y = (int)limits.maxComputeWorkGroupCount[1],
      .max_compute_work_group_count_z = (int)limits.maxComputeWorkGroupCount[2],
      .max_compute_work_group_size_x = (int)limits.maxComputeWorkGroupSize[0],
      .max_compute_work_group_size_y = (int)limits.maxComputeWorkGroupSize[1],
      .max_compute_work_group_size_z = (int)limits.maxComputeWorkGroupSize[2],
      .max_compute_uniform_components = 1024,
      .max_compute_texture_image_units = 16,
      .max_compute_image_uniforms = 8,
      .max_compute_atomic_counters = 8,
      .max_compute_atomic_counter_buffers = 1,
      .max_varying_components = 60,
      .max_vertex_output_components = (int)limits.maxVertexOutputComponents,
      .max_geometry_input_components = (int)limits.maxGeometryInputComponents,
      .max_geometry_output_components = (int)limits.maxGeometryOutputComponents,
      .max_fragment_input_components = (int)limits.maxFragmentInputComponents,
      .max_image_units = 8,
      .max_combined_image_units_and_fragment_outputs = 8,
      .max_combined_shader_output_resources = 8,
      .max_image_samples = 0,
      .max_vertex_image_uniforms = 0,
      .max_tess_control_image_uniforms = 0,
      .max_tess_evaluation_image_uniforms = 0,
      .max_geometry_image_uniforms = 0,
      .max_fragment_image_uniforms = 8,
      .max_combined_image_uniforms = 8,
      .max_geometry_texture_image_units = 16,
      .max_geometry_output_vertices = (int)limits.maxGeometryOutputVertices,
      .max_geometry_total_output_components =
          (int)limits.maxGeometryTotalOutputComponents,
      .max_geometry_uniform_components = 1024,
      .max_geometry_varying_components = 64,
      .max_tess_control_input_components =
          (int)limits.maxTessellationControlPerVertexInputComponents,
      .max_tess_control_output_components =
          (int)limits.maxTessellationControlPerVertexOutputComponents,
      .max_tess_control_texture_image_units = 16,
      .max_tess_control_uniform_components = 1024,
      .max_tess_control_total_output_components = 4096,
      .max_tess_evaluation_input_components =
          (int)limits.maxTessellationEvaluationInputComponents,
      .max_tess_evaluation_output_components =
          (int)limits.maxTessellationEvaluationOutputComponents,
      .max_tess_evaluation_texture_image_units = 16,
      .max_tess_evaluation_uniform_components = 1024,
      .max_tess_patch_components = 120,
      .max_patch_vertices = 32,
      .max_tess_gen_level = 64,
      .max_viewports = (int)limits.maxViewports,
      .max_vertex_atomic_counters = 0,
      .max_tess_control_atomic_counters = 0,
      .max_tess_evaluation_atomic_counters = 0,
      .max_geometry_atomic_counters = 0,
      .max_fragment_atomic_counters = 8,
      .max_combined_atomic_counters = 8,
      .max_atomic_counter_bindings = 1,
      .max_vertex_atomic_counter_buffers = 0,
      .max_tess_control_atomic_counter_buffers = 0,
      .max_tess_evaluation_atomic_counter_buffers = 0,
      .max_geometry_atomic_counter_buffers = 0,
      .max_fragment_atomic_counter_buffers = 1,
      .max_combined_atomic_counter_buffers = 1,
      .max_atomic_counter_buffer_size = 16384,
      .max_transform_feedback_buffers = 4,
      .max_transform_feedback_interleaved_components = 64,
      .max_cull_distances = (int)limits.maxCullDistances,
      .max_combined_clip_and_cull_distances =
          (int)limits.maxCombinedClipAndCullDistances,
      .max_samples = 4,
      .max_mesh_output_vertices_nv = 256,
      .max_mesh_output_primitives_nv = 512,
      .max_mesh_work_group_size_x_nv = 32,
      .max_mesh_work_group_size_y_nv = 1,
      .max_mesh_work_group_size_z_nv = 1,
      .max_task_work_group_size_x_nv = 32,
      .max_task_work_group_size_y_nv = 1,
      .max_task_work_group_size_z_nv = 1,
      .max_mesh_view_count_nv = 4,
      .max_mesh_output_vertices_ext = 256,
      .max_mesh_output_primitives_ext = 512,
      .max_mesh_work_group_size_x_ext = 32,
      .max_mesh_work_group_size_y_ext = 1,
      .max_mesh_work_group_size_z_ext = 1,
      .max_task_work_group_size_x_ext = 32,
      .max_task_work_group_size_y_ext = 1,
      .max_task_work_group_size_z_ext = 1,
      .max_mesh_view_count_ext = 4,
      .maxDualSourceDrawBuffersEXT = 1,
      .limits =
          {
              .non_inductive_for_loops = true,
              .while_loops = true,
              .do_while_loops = true,
              .general_uniform_indexing = true,
              .general_attribute_matrix_vector_indexing = true,
              .general_varying_indexing = true,
              .general_sampler_indexing = true,
              .general_variable_indexing = true,
              .general_constant_matrix_vector_indexing = true,
          },
  };

  return resource;
}

glslang_stage_t getGLSLangShaderStage(ShaderStage stage) {
  switch (stage) {
  case ShaderStage::Vert:
    return GLSLANG_STAGE_VERTEX;
  case ShaderStage::Tece:
    return GLSLANG_STAGE_TESSCONTROL;
  case ShaderStage::Tese:
    return GLSLANG_STAGE_TESSEVALUATION;
  case ShaderStage::Geom:
    return GLSLANG_STAGE_GEOMETRY;
  case ShaderStage::Frag:
    return GLSLANG_STAGE_FRAGMENT;
  case ShaderStage::Comp:
    return GLSLANG_STAGE_COMPUTE;
  }

  assert(false);
}

void compileShaderGlslang(ShaderStage stage, const char *code,
                          std::vector<uint8_t> *outSPIRV,
                          const glslang_resource_t *glslLangResource) {
  if (!outSPIRV) {
    std::cerr << "outSpriv is null" << std::endl;
    assert(false);
  }

  const glslang_input_t input = {
      .language = GLSLANG_SOURCE_GLSL,
      .stage = getGLSLangShaderStage(stage),
      .client = GLSLANG_CLIENT_VULKAN,
      .client_version = GLSLANG_TARGET_VULKAN_1_3,
      .target_language = GLSLANG_TARGET_SPV,
      .target_language_version = GLSLANG_TARGET_SPV_1_6,
      .code = code,
      .default_version = 100,
      .default_profile = GLSLANG_NO_PROFILE,
      .force_default_version_and_profile = false,
      .forward_compatible = false,
      .messages = GLSLANG_MSG_DEFAULT_BIT,
      .resource = glslLangResource,
  };

  glslang_shader_t *shader = glslang_shader_create(&input);

  if (!glslang_shader_preprocess(shader, &input)) {
    std::cerr << "Shader preprocessing failed:\n" << std::endl;
    printf("  %s\n", glslang_shader_get_info_log(shader));
    printf("  %s\n", glslang_shader_get_info_debug_log(shader));
    std::cout << code << std::endl;
    assert(false);
  }

  if (!glslang_shader_parse(shader, &input)) {
    std::cerr << "Shader parsing failed:\n" << std::endl;
    printf("  %s\n", glslang_shader_get_info_log(shader));
    printf("  %s\n", glslang_shader_get_info_debug_log(shader));
    std::cout << glslang_shader_get_preprocessed_code(shader) << std::endl;
    assert(false);
  }

  glslang_program_t *program = glslang_program_create();
  glslang_program_add_shader(program, shader);

  if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT |
                                         GLSLANG_MSG_VULKAN_RULES_BIT)) {
    std::cerr << "Shader linking failed:\n" << std::endl;
    printf("  %s\n", glslang_program_get_info_log(program));
    printf("  %s\n", glslang_program_get_info_debug_log(program));
    assert(false);
  }

  glslang_spv_options_t options = {
      .generate_debug_info = true,
      .strip_debug_info = false,
      .disable_optimizer = false,
      .optimize_size = true,
      .disassemble = false,
      .validate = true,
      .emit_nonsemantic_shader_debug_info = false,
      .emit_nonsemantic_shader_debug_source = false,
  };

  glslang_program_SPIRV_generate_with_options(program, input.stage, &options);

  if (glslang_program_SPIRV_get_messages(program)) {
    printf("%s\n", glslang_program_SPIRV_get_messages(program));
  }

  const uint8_t *spirv =
      reinterpret_cast<const uint8_t *>(glslang_program_SPIRV_get_ptr(program));
  const size_t numBytes =
      glslang_program_SPIRV_get_size(program) * sizeof(uint32_t);

  *outSPIRV = std::vector(spirv, spirv + numBytes);

  glslang_program_delete(program);
  glslang_shader_delete(shader);
}
#endif
}; // namespace MAI
#endif
