#include "window.hpp"

#include "neuron/logutil.hpp"

namespace neuron {
    std::atomic_int Window::window_counter = 0;

    Window::Window(const Attributes &attributes) {
        NEURON_TRACE_TOOL();
        if (window_counter++ == 0) {
            glfwInit();
        }

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // TODO: video modes & resizing
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window = glfwCreateWindow(attributes.size.width, attributes.size.height, attributes.title.c_str(), nullptr, nullptr);

        if (!m_window) {
            throw std::runtime_error("Failed to create GLFW window");
        }
    }

    Window::~Window() {
        NEURON_TRACE_TOOL();

        glfwDestroyWindow(m_window);

        if (--window_counter == 0) {
            glfwTerminate();
        }
    }

    std::unique_ptr<vk::raii::SurfaceKHR> Window::create_surface(const std::shared_ptr<vk::raii::Instance> &instance) const {
        NEURON_TRACE_TOOL();
        VkSurfaceKHR surf;
        if (glfwCreateWindowSurface(**instance, m_window, nullptr, &surf) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }

        return std::make_unique<vk::raii::SurfaceKHR>(*instance, surf);
    }


    vk::Extent2D Window::size() const {
        NEURON_TRACE_TOOL();
        glm::ivec2 s;
        glfwGetFramebufferSize(m_window, &s.x, &s.y);
        return {static_cast<uint32_t>(s.x), static_cast<uint32_t>(s.y)};
    }

    bool Window::should_close() const {
        NEURON_TRACE_TOOL();
        return glfwWindowShouldClose(m_window);
    }

    void Window::close() const {
        NEURON_TRACE_TOOL();
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

    double Window::time() {
        NEURON_TRACE_TOOL();
        return glfwGetTime();
    }

    void Window::poll() {
        NEURON_TRACE_TOOL();
        glfwPollEvents();
    }
} // namespace neuron
