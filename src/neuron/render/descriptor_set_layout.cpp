#include "descriptor_set_layout.hpp"

namespace neuron::render {

    DescriptorSetLayout::DescriptorSetLayout(const std::shared_ptr<vk::raii::Device> &device, const Settings &settings) : m_device(device) {
        m_descriptor_set_layout = vk::raii::DescriptorSetLayout(
            *m_device, vk::DescriptorSetLayoutCreateInfo(settings.update_after_bind_pool ? vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool
                                                                                         : vk::DescriptorSetLayoutCreateFlags{},
                                                         settings.bindings));
    }
} // namespace neuron::render
