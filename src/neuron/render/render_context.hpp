#pragma once
#include "shader_module.hpp"

#include <memory>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <vk_mem_alloc.h>

namespace neuron {
    class Window;
}

namespace neuron::render {
    struct SurfaceConfiguration {
        vk::Extent2D       image_extent;
        vk::Format         image_format;
        vk::ColorSpaceKHR  image_color_space;
        vk::PresentModeKHR present_mode;

        std::vector<vk::Image>                            images;
        std::vector<std::shared_ptr<vk::raii::ImageView>> image_views;

        SurfaceConfiguration(const SurfaceConfiguration &other)                = delete;
        SurfaceConfiguration(SurfaceConfiguration &&other) noexcept            = default;
        SurfaceConfiguration &operator=(const SurfaceConfiguration &other)     = delete;
        SurfaceConfiguration &operator=(SurfaceConfiguration &&other) noexcept = default;

        SurfaceConfiguration()
            : image_format(vk::Format::eUndefined), image_color_space(static_cast<vk::ColorSpaceKHR>(VK_COLOR_SPACE_MAX_ENUM_KHR)),
              present_mode(vk::PresentModeKHR::eFifo) {};
    };

    struct FrameInfo {
        uint32_t                   image_index;
        const vk::Image           &image;
        const vk::raii::ImageView &image_view;
        const vk::raii::Semaphore &image_available_semaphore;
        const vk::raii::Semaphore &render_finished_semaphore;
        const vk::raii::Fence     &in_flight_fence;

        FrameInfo(const FrameInfo &other)                = delete;
        FrameInfo(FrameInfo &&other) noexcept            = default;
        FrameInfo &operator=(const FrameInfo &other)     = delete;
        FrameInfo &operator=(FrameInfo &&other) noexcept = default;

        FrameInfo(const uint32_t imageIndex, const vk::Image &image, const vk::raii::ImageView &imageView,
                  const vk::raii::Semaphore &imageAvailableSemaphore, const vk::raii::Semaphore &renderFinishedSemaphore,
                  const vk::raii::Fence &inFlightFence)
            : image_index(imageIndex), image(image), image_view(imageView), image_available_semaphore(imageAvailableSemaphore),
              render_finished_semaphore(renderFinishedSemaphore), in_flight_fence(inFlightFence) {}
    };

    class RenderContext final {
      public:
        static constexpr uint64_t FRAME_TIMEOUT = 4e9; // wait 4 seconds for a frame before giving up

        explicit RenderContext(const std::shared_ptr<Window> &window /* TODO: parameters */);
        ~RenderContext();

        inline vk::raii::Context const &raii() const { return m_raii_context; };

        [[nodiscard]] std::shared_ptr<vk::raii::Instance> instance() const { return m_instance; }

        [[nodiscard]] std::shared_ptr<vk::raii::PhysicalDevice> physical_device() const { return m_physical_device; }

        [[nodiscard]] const std::unique_ptr<vk::raii::SurfaceKHR> &surface() const { return m_surface; }

        [[nodiscard]] std::shared_ptr<vk::raii::Device> device() const { return m_device; }

        [[nodiscard]] std::shared_ptr<vk::raii::Queue> queue() const { return m_queue; }

        [[nodiscard]] std::optional<unsigned int> queue_family() const { return m_queue_family; }

        [[nodiscard]] const std::unique_ptr<vk::raii::SwapchainKHR> &swapchain() const { return m_swapchain; }

        [[nodiscard]] const std::optional<SurfaceConfiguration> &surface_configuration() const { return m_surface_configuration; }

        FrameInfo begin_frame();
        void      end_frame(FrameInfo frame_info);

        [[nodiscard]] inline std::shared_ptr<ShaderModule> load_shader(const std::filesystem::path &file_path) const {
            return ShaderModule::load(m_device, file_path);
        }

        [[nodiscard]] vk::Viewport viewport_full(float min_depth, float max_depth) const;
        [[nodiscard]] vk::Rect2D   scissor_full() const;

        [[nodiscard]] VmaAllocator allocator() const { return m_allocator; }

      private:
        vk::raii::Context                         m_raii_context;
        std::shared_ptr<vk::raii::Instance>       m_instance;
        std::shared_ptr<vk::raii::PhysicalDevice> m_physical_device;
        std::unique_ptr<vk::raii::SurfaceKHR>     m_surface;
        std::shared_ptr<vk::raii::Device>         m_device;
        std::shared_ptr<vk::raii::Queue>          m_queue; // TODO: grab different queues instead of just crashing
        std::optional<unsigned int>               m_queue_family;

        std::unique_ptr<vk::raii::SwapchainKHR> m_swapchain;
        std::unique_ptr<vk::raii::Fence>        m_in_flight_fence;
        std::unique_ptr<vk::raii::Semaphore>    m_image_available_semaphore;
        std::unique_ptr<vk::raii::Semaphore>    m_render_finished_semaphore;

        std::optional<SurfaceConfiguration> m_surface_configuration;
        const std::shared_ptr<Window>      &m_window;

        VmaAllocator m_allocator;

        void setup_swapchain();
    };

} // namespace neuron::render
