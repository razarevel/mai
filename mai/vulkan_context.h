#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

#ifdef MAI_USE_VMA
#include <vk_mem_alloc.h>
#endif

struct QueueFamilyIndices {
  std::optional<uint32_t> graphcisFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() const {
    return graphcisFamily.has_value() && presentFamily.has_value();
  }
};

namespace MAI {
struct VulkanContext {
  VulkanContext(GLFWwindow *window, const char *name);
  ~VulkanContext();

private:
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

  void createInstance();
  void setupDebugger();
  void createSurfaceKHR();
  void pickPhysicalDevice();
  void createLogicalDevice();

#ifdef MAI_USE_VMA
  void createVmaAllocation();
#endif
};
}; // namespace MAI
