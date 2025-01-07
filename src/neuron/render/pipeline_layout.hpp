#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace neuron::render {
    class DescriptorSetLayout;

    class PipelineLayout final {
      public:
        struct Settings {
            std::vector<vk::PushConstantRange> push_constant_ranges;
            std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts;
        };

        PipelineLayout(const std::shared_ptr<vk::raii::Device> &device, const Settings &settings);
        ~PipelineLayout() = default;

        inline const vk::raii::PipelineLayout &get() const noexcept { return m_pipeline_layout; }

        inline const vk::raii::PipelineLayout &operator*() const noexcept { return m_pipeline_layout; }

        inline const vk::raii::PipelineLayout *operator->() const noexcept { return &m_pipeline_layout; }

      private:
        std::shared_ptr<vk::raii::Device> m_device;
        vk::raii::PipelineLayout          m_pipeline_layout = nullptr;
    };

} // namespace neuron::render
