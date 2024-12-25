#pragma once

#include <memory>
#include <vulkan/vulkan_raii.hpp>

namespace neuron::render {

    class DescriptorSetLayout {
      public:
        struct Settings {
            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            bool update_after_bind_pool = false;
        };

        DescriptorSetLayout(const std::shared_ptr<vk::raii::Device> &device, const Settings &settings);

        DescriptorSetLayout(const DescriptorSetLayout &other)                = delete;
        DescriptorSetLayout(DescriptorSetLayout &&other) noexcept            = default;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &other)     = delete;
        DescriptorSetLayout &operator=(DescriptorSetLayout &&other) noexcept = default;

        inline const vk::raii::DescriptorSetLayout &get() const noexcept { return m_descriptor_set_layout; };

        inline const vk::raii::DescriptorSetLayout &operator*() const noexcept { return m_descriptor_set_layout; };

        inline const vk::raii::DescriptorSetLayout *operator->() const { return &m_descriptor_set_layout; };

      private:
        std::shared_ptr<vk::raii::Device> m_device;
        vk::raii::DescriptorSetLayout     m_descriptor_set_layout = nullptr;
    };

} // namespace neuron::render
