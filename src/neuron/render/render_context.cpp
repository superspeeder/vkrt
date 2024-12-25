#define VMA_IMPLEMENTATION

#include "render_context.hpp"

#include "neuron/logutil.hpp"
#include "neuron/window.hpp"

#include <spdlog/spdlog.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace neuron::render {
    struct Features {
        vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan12Features,
                           vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
                           vk::PhysicalDeviceRayQueryFeaturesKHR, vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR,
                           vk::PhysicalDeviceRayTracingPipelineFeaturesKHR, 
                           vk::PhysicalDeviceShaderClockFeaturesKHR>
            f;
    };

    Features required_features() {
        NEURON_TRACE_TOOL();

        decltype(Features::f) features{};
        {
            auto &f = features.get<vk::PhysicalDeviceFeatures2>().features;

            f.sparseBinding = true;
        }
        {
            auto &f = features.get<vk::PhysicalDeviceVulkan11Features>();

            f.variablePointers              = true;
            f.variablePointersStorageBuffer = true;
            f.shaderDrawParameters          = true;
        }
        {
            auto &f = features.get<vk::PhysicalDeviceVulkan12Features>();

            f.descriptorIndexing                       = true;
            f.timelineSemaphore                        = true;
            f.uniformBufferStandardLayout              = true;
            f.descriptorBindingVariableDescriptorCount = true;
        }
        {
            auto &f              = features.get<vk::PhysicalDeviceVulkan13Features>();
            f.maintenance4       = true;
            f.synchronization2   = true;
            f.dynamicRendering   = true;
            f.inlineUniformBlock = true;
        }
        {
            auto &f                 = features.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>();
            f.accelerationStructure = true;
        }
        {
            auto &f                               = features.get<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>();
            f.rayTracingPipeline                  = true;
            f.rayTraversalPrimitiveCulling        = true;
            f.rayTracingPipelineTraceRaysIndirect = true;
        }
        {
            auto &f               = features.get<vk::PhysicalDeviceShaderClockFeaturesKHR>();
            f.shaderDeviceClock   = true;
            f.shaderSubgroupClock = true;
        }
        {
            auto &f    = features.get<vk::PhysicalDeviceRayQueryFeaturesKHR>();
            f.rayQuery = true;
        }
        {
            auto &f                                = features.get<vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR>();
            f.rayTracingMaintenance1               = true;
            f.rayTracingPipelineTraceRaysIndirect2 = true;
        }
        return {features};
    }

    RenderContext::RenderContext(const std::shared_ptr<Window> &window) : m_window(window) {
        NEURON_TRACE_TOOL();
        VULKAN_HPP_DEFAULT_DISPATCHER.init();

        uint32_t   instext_count;
        const auto required_instance_extensions = glfwGetRequiredInstanceExtensions(&instext_count);

        constexpr vk::ApplicationInfo app_info{"", vk::makeApiVersion(0, 1, 0, 0), "Neuron", vk::makeApiVersion(0, 1, 0, 0), vk::ApiVersion13};

        m_instance = std::make_shared<vk::raii::Instance>(raii(),
                                                          vk::InstanceCreateInfo({}, &app_info, 0, {}, instext_count, required_instance_extensions));

        VULKAN_HPP_DEFAULT_DISPATCHER.init(**m_instance);

        auto physical_devices = m_instance->enumeratePhysicalDevices();
        m_physical_device     = std::make_shared<vk::raii::PhysicalDevice>(physical_devices[0]);

        m_surface = m_window->create_surface(m_instance);

        const auto queue_family_properties = m_physical_device->getQueueFamilyProperties();
        for (uint32_t i = 0; i < queue_family_properties.size(); ++i) {
            if (const auto &props = queue_family_properties[i]; props.queueFlags & vk::QueueFlagBits::eGraphics &&
                                                                props.queueFlags & vk::QueueFlagBits::eCompute &&
                                                                m_physical_device->getSurfaceSupportKHR(i, **m_surface)) {
                m_queue_family = i;
                break;
            }
        }

        if (!m_queue_family.has_value()) {
            throw std::runtime_error("Failed to find valid queue family");
        }

        constexpr float           prio = 1.0f;
        vk::DeviceQueueCreateInfo queue_create_info(vk::DeviceQueueCreateFlags{}, *m_queue_family, 1, &prio);

        auto features = required_features().f;

        auto extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_SHADER_CLOCK_EXTENSION_NAME,
            VK_KHR_RAY_QUERY_EXTENSION_NAME,
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME,
        };

        m_device = std::make_shared<vk::raii::Device>(
            *m_physical_device, vk::DeviceCreateInfo({}, queue_create_info, {}, extensions, nullptr, &features.get<vk::PhysicalDeviceFeatures2>()));

        VULKAN_HPP_DEFAULT_DISPATCHER.init(**m_device);

        m_queue = std::make_shared<vk::raii::Queue>(m_device->getQueue(m_queue_family.value(), 0));

        m_in_flight_fence = std::make_unique<vk::raii::Fence>(*m_device, vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));

        m_image_available_semaphore = std::make_unique<vk::raii::Semaphore>(*m_device, vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags()));
        m_render_finished_semaphore = std::make_unique<vk::raii::Semaphore>(*m_device, vk::SemaphoreCreateInfo(vk::SemaphoreCreateFlags()));

        setup_swapchain();

        {
            VmaAllocatorCreateInfo aci{};
            aci.instance = **m_instance;
            aci.physicalDevice = **m_physical_device;
            aci.device = **m_device;
            aci.vulkanApiVersion = vk::ApiVersion13;
            if (vmaCreateAllocator(&aci, &m_allocator) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create vma allocator");
            }
        }
    }

    RenderContext::~RenderContext() {
        NEURON_TRACE_TOOL();

        vmaDestroyAllocator(m_allocator);
        m_allocator = nullptr;
    }

    void RenderContext::setup_swapchain() {
        NEURON_TRACE_TOOL();

        const auto caps          = m_physical_device->getSurfaceCapabilitiesKHR(*m_surface);
        const auto formats       = m_physical_device->getSurfaceFormatsKHR(*m_surface);
        const auto present_modes = m_physical_device->getSurfacePresentModesKHR(*m_surface);

        SurfaceConfiguration surface_configuration{};

        surface_configuration.image_extent = caps.currentExtent;
        if (surface_configuration.image_extent.width == UINT32_MAX) {
            const auto window_extent           = m_window->size();
            surface_configuration.image_extent = vk::Extent2D(
                std::clamp(window_extent.width, caps.minImageExtent.width, caps.maxImageExtent.width),
                std::clamp(window_extent.height, caps.minImageExtent.height, caps.maxImageExtent.height));
        }

        auto surface_format = formats[0];
        for (const auto &format : formats) {
            if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                surface_format = format;
                break;
            }
        }

        surface_configuration.image_format      = surface_format.format;
        surface_configuration.image_color_space = surface_format.colorSpace;

        surface_configuration.present_mode = vk::PresentModeKHR::eFifo;
        for (const auto &pm : present_modes) {
            if (pm == vk::PresentModeKHR::eMailbox) {
                surface_configuration.present_mode = pm;
            }
        }

        vk::SwapchainCreateInfoKHR create_info{};
        create_info.imageExtent     = surface_configuration.image_extent;
        create_info.imageFormat     = surface_configuration.image_format;
        create_info.imageColorSpace = surface_configuration.image_color_space;
        create_info.presentMode     = surface_configuration.present_mode;
        create_info.oldSwapchain    = m_swapchain == nullptr ? nullptr : **m_swapchain;
        create_info.surface         = *m_surface;
        create_info.minImageCount   = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && caps.minImageCount > caps.maxImageCount) {
            create_info.minImageCount = caps.maxImageCount;
        }
        create_info.clipped          = true;
        create_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        create_info.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
        create_info.imageSharingMode = vk::SharingMode::eExclusive;
        create_info.preTransform     = caps.currentTransform;
        create_info.imageArrayLayers = 1;
        create_info.setQueueFamilyIndices({});

        m_device->waitIdle();
        m_surface_configuration = std::move(surface_configuration);

        m_swapchain                     = std::make_unique<vk::raii::SwapchainKHR>(*m_device, create_info);
        m_surface_configuration->images = m_swapchain->getImages();

        m_surface_configuration->image_views.reserve(m_surface_configuration->images.size());
        for (const auto &img : m_surface_configuration->images) {
            m_surface_configuration->image_views.push_back(std::make_shared<vk::raii::ImageView>(
                *m_device, vk::ImageViewCreateInfo({}, img, vk::ImageViewType::e2D, m_surface_configuration->image_format,
                                                   vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB,
                                                                        vk::ComponentSwizzle::eA),
                                                   vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))));
        }

        spdlog::info("Created swapchain with {} images", m_surface_configuration->images.size());
        spdlog::debug("Swapchain present mode: {}", vk::to_string(m_surface_configuration->present_mode));
        spdlog::debug("Swapchain format: {}", vk::to_string(m_surface_configuration->image_format));
        spdlog::debug("Swapchain color space: {}", vk::to_string(m_surface_configuration->image_color_space));
        spdlog::debug("Swapchain extent: {}x{}", m_surface_configuration->image_extent.width, m_surface_configuration->image_extent.height);
    }

    FrameInfo RenderContext::begin_frame() {
        NEURON_TRACE_TOOL();

        if (m_device->waitForFences(**m_in_flight_fence, true, FRAME_TIMEOUT) == vk::Result::eTimeout) {
            throw std::runtime_error("Timed out waiting for previous frame to finish rendering.");
        }

        auto [result, image_index] = m_swapchain->acquireNextImage(FRAME_TIMEOUT, **m_image_available_semaphore, nullptr);
        if (result == vk::Result::eTimeout) {
            throw std::runtime_error("Timed out waiting for next image.");
        }

        m_device->resetFences(**m_in_flight_fence);

        return FrameInfo(image_index, m_surface_configuration->images[image_index], *m_surface_configuration->image_views[image_index],
                         *m_image_available_semaphore, *m_render_finished_semaphore, *m_in_flight_fence);
    }

    void RenderContext::end_frame(FrameInfo frame_info) {
        NEURON_TRACE_TOOL();

        vk::PresentInfoKHR pi{};
        pi.setSwapchains(**m_swapchain);
        pi.setImageIndices(frame_info.image_index);
        pi.setWaitSemaphores(*frame_info.render_finished_semaphore);

        try {
            if (m_queue->presentKHR(pi) == vk::Result::eSuboptimalKHR) {
                spdlog::info("Swapchain is suboptimal, recreating.");
                setup_swapchain();
            }
        } catch (vk::OutOfDateKHRError &_) {
            spdlog::info("Swapchain is out of date, recreating.");
            setup_swapchain();
        }
    }

    vk::Viewport RenderContext::viewport_full(float min_depth, float max_depth) const {
        return {
            0.0f,
            0.0f,
            static_cast<float>(m_surface_configuration->image_extent.width),
            static_cast<float>(m_surface_configuration->image_extent.height),
            min_depth,
            max_depth,
        };
    }

    vk::Rect2D RenderContext::scissor_full() const {
        return {
            {0, 0},
            m_surface_configuration->image_extent,
        };
    }
} // namespace neuron::render
