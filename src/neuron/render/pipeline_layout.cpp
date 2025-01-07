#include "pipeline_layout.hpp"

#include "descriptor_set_layout.hpp"

namespace neuron::render {
    PipelineLayout::PipelineLayout(const std::shared_ptr<vk::raii::Device> &device, const Settings &settings) : m_device(device) {
        std::vector<vk::DescriptorSetLayout> set_layouts;
        set_layouts.reserve(settings.descriptor_set_layouts.size());
        for (const auto &dsl : settings.descriptor_set_layouts) {
            set_layouts.push_back(***dsl);
        }
        m_pipeline_layout = vk::raii::PipelineLayout(*m_device, vk::PipelineLayoutCreateInfo({}, set_layouts, settings.push_constant_ranges));
    }
} // namespace neuron::render
