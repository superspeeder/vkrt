#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace neuron::render {
    class DescriptorSetLayout;
}

namespace neuron::render {
    class DescriptorPool {
      public:
        DescriptorPool(const std::shared_ptr<vk::raii::Device> &device, std::vector<vk::DescriptorPoolSize> pool_sizes, uint32_t max_sets);

        DescriptorPool(const DescriptorPool &other)                = delete;
        DescriptorPool(DescriptorPool &&other) noexcept            = default;
        DescriptorPool &operator=(const DescriptorPool &other)     = delete;
        DescriptorPool &operator=(DescriptorPool &&other) noexcept = default;

        inline const vk::raii::DescriptorPool &get() const noexcept { return m_descriptor_pool; }
        inline const vk::raii::DescriptorPool &operator*() const noexcept { return m_descriptor_pool; }
        inline const vk::raii::DescriptorPool *operator->() const noexcept { return &m_descriptor_pool; }

        void reset() const;
        vk::raii::DescriptorSets allocate(std::vector<vk::DescriptorSetLayout> layouts) const;
        vk::raii::DescriptorSets allocate(std::vector<std::shared_ptr<DescriptorSetLayout>> layouts) const;

      private:
        std::shared_ptr<vk::raii::Device> m_device;
        vk::raii::DescriptorPool m_descriptor_pool = nullptr;
    };

} // namespace neuron::render
