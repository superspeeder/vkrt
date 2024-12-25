#include "graphics_pipeline.hpp"

namespace neuron::render {
    GraphicsPipeline::GraphicsPipeline(const std::shared_ptr<vk::raii::Device> &device, const Settings &settings)
        : m_device(device), m_layout(settings.layout), m_render_pass(settings.render_pass) {
        std::vector<vk::PipelineShaderStageCreateInfo> stages;
        for (const auto &[module, stage, entry] : settings.shaders) {
            stages.push_back(vk::PipelineShaderStageCreateInfo({}, stage, **module, entry.c_str()));
            m_shader_modules.push_back(module);
        }

        vk::PipelineVertexInputStateCreateInfo   vertex_input_state{{}, settings.vertex_bindings, settings.vertex_attributes};
        vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{{}, settings.topology, false};
        vk::PipelineTessellationStateCreateInfo  tessellation_state{{}, 1};
        vk::PipelineDynamicStateCreateInfo       dynamic_state{{}, settings.dynamic_states};
        vk::PipelineViewportStateCreateInfo      viewport_state{{}, settings.viewport, settings.scissor};
        vk::PipelineRasterizationStateCreateInfo rasterization_state{
            {}, false, false, settings.polygon_mode, settings.cull_mode, settings.front_face, false, 0.0f, 0.0f, 0.0f, settings.line_width};
        vk::PipelineMultisampleStateCreateInfo  multisample_state{{}, vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false};
        vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{{}, false, false, vk::CompareOp::eNever, false, false, {}, {}, 0.0f, 1.0f};
        vk::PipelineColorBlendStateCreateInfo color_blend_state{{}, false, vk::LogicOp::eClear, settings.color_attachments, settings.blend_constants};

        vk::RenderPass render_pass = m_render_pass == nullptr ? nullptr : **m_render_pass;

        std::optional<vk::PipelineRenderingCreateInfo> rendering_info;
        if (settings.rendering_attachment_formats.has_value()) {
            rendering_info = vk::PipelineRenderingCreateInfo(0, settings.rendering_attachment_formats->color_attachments,
                                                             settings.rendering_attachment_formats->depth_attachment,
                                                             settings.rendering_attachment_formats->stencil_attachment);
        }

        m_pipeline = vk::raii::Pipeline(*m_device, nullptr,
                                        vk::GraphicsPipelineCreateInfo({}, stages, &vertex_input_state, &input_assembly_state, &tessellation_state,
                                                                       &viewport_state, &rasterization_state, &multisample_state,
                                                                       &depth_stencil_state, &color_blend_state, &dynamic_state, **m_layout,
                                                                       render_pass, settings.subpass_index, nullptr, 0,
                                                                       rendering_info.has_value() ? &(rendering_info.value()) : nullptr));
    }
} // namespace neuron::render
