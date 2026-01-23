#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

namespace MAI {

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

// void compileShaderGlslang(ShaderStage stage, const char *code,
//                           std::vector<uint8_t> *outSPIRV,
//                           const glslang_resource_t *glslLangResource);
}; // namespace MAI
