// #include "mai.h"
//
// #include <cassert>
// #include <stdexcept>
//
// namespace MAI {
//
// GLFWwindow *initWindow(WindowInfo info) {
//   if (!glfwInit())
//     throw std::runtime_error("failed to init glfw");
//
//   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//   if (!info.allowResize)
//     glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//
//   GLFWwindow *window = glfwCreateWindow(
//       info.width, info.height, info.appName,
//       info.isFullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
//
//   glfwSetKeyCallback(window,
//                      [](auto *window, int keys, int, int action, int mods) {
//                        if (keys == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//                          glfwSetWindowShouldClose(window, GLFW_TRUE);
//                      });
//
//   return window;
// }
//
// Renderer *initVulkanWithSwapChain(GLFWwindow *window, const char *appName) {
//   assert(window);
//   assert(appName != nullptr);
//   VulkanContext *ctx = new VulkanContext(window, appName);
//   Renderer *ren = new Renderer(ctx);
//
//   return ren;
// }
//
// Renderer::~Renderer() { delete ctx; }
// }; // namespace MAI
