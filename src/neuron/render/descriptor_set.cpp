#include "descriptor_set.hpp"
#include "neuron/render/descriptor_set_layout.hpp"

namespace neuron::render {
    DescriptorPool::DescriptorPool(const std::shared_ptr<vk::raii::Device> &device, std::vector<vk::DescriptorPoolSize> pool_sizes,
                                   const uint32_t max_sets)
        : m_device(device) {
        m_descriptor_pool = m_device->createDescriptorPool(vk::DescriptorPoolCreateInfo{{}, max_sets, pool_sizes});
    }

    void DescriptorPool::reset() const {
        m_descriptor_pool.reset();
    }

    vk::raii::DescriptorSets DescriptorPool::allocate(std::vector<vk::DescriptorSetLayout> layouts) const {
        return vk::raii::DescriptorSets(*m_device, vk::DescriptorSetAllocateInfo(*m_descriptor_pool, layouts));
    }

    vk::raii::DescriptorSets DescriptorPool::allocate(std::vector<std::shared_ptr<DescriptorSetLayout>> layouts) const {
        std::vector<vk::DescriptorSetLayout> set_layouts;
        set_layouts.reserve(layouts.size());
        for (const auto &dsl : layouts) {
            set_layouts.push_back(***dsl);
        }
        return allocate(set_layouts);
    }
} // namespace neuron::render
