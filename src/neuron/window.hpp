#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

namespace neuron {
    class Window final {
      public:
        struct Attributes {
            vk::Extent2D size{800, 600};
            std::string  title = "Window";
        };

        explicit Window(const Attributes &attributes = {});
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        Window(Window &&) = default;
        Window &operator=(Window &&) = default;


        std::unique_ptr<vk::raii::SurfaceKHR> create_surface(const std::shared_ptr<vk::raii::Instance> &instance) const;

        vk::Extent2D size() const;
        bool         should_close() const;
        void         close() const;

        static double time();
        static void poll();

      private:
        GLFWwindow *m_window;

        static std::atomic_int window_counter;
    };

} // namespace neuron
