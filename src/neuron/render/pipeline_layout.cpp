#include "pipeline_layout.hpp"

namespace neuron::render {
    PipelineLayout::PipelineLayout(const std::shared_ptr<vk::raii::Device> &device, const Settings &settings) : m_device(device) {
        m_pipeline_layout = vk::raii::PipelineLayout(*m_device, vk::PipelineLayoutCreateInfo());
    }
} // namespace neuron::render
