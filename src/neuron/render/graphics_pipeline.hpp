#pragma once

#include "pipeline_layout.hpp"
#include "shader_module.hpp"


#include <vulkan/vulkan_raii.hpp>

namespace neuron::render {

    struct ShaderStage {
        std::shared_ptr<ShaderModule> module;
        vk::ShaderStageFlagBits       stage;
        std::string                   entry = "main";
    };

    struct RenderingAttachmentFormats {
        std::vector<vk::Format> color_attachments;
        vk::Format depth_attachment = vk::Format::eUndefined;
        vk::Format stencil_attachment = vk::Format::eUndefined;
    };

    class GraphicsPipeline final {
      public:
        struct Settings {
            std::vector<vk::VertexInputBindingDescription>     vertex_bindings;
            std::vector<vk::VertexInputAttributeDescription>   vertex_attributes;
            std::vector<vk::DynamicState>                      dynamic_states;
            std::vector<ShaderStage>                           shaders;
            std::vector<vk::PipelineColorBlendAttachmentState> color_attachments;

            std::array<float, 4>  blend_constants{0.0f, 0.0f, 0.0f, 0.0f};
            vk::PrimitiveTopology topology     = vk::PrimitiveTopology::eTriangleList;
            vk::PolygonMode       polygon_mode = vk::PolygonMode::eFill;
            float                 line_width   = 1.0f;
            vk::CullModeFlags     cull_mode    = vk::CullModeFlagBits::eBack;
            vk::FrontFace         front_face   = vk::FrontFace::eClockwise;

            vk::Viewport viewport;
            vk::Rect2D   scissor;

            std::shared_ptr<PipelineLayout> layout;

            std::shared_ptr<vk::raii::RenderPass> render_pass   = nullptr;
            uint32_t                              subpass_index = 0;

            std::optional<RenderingAttachmentFormats> rendering_attachment_formats;
        };

        GraphicsPipeline(const std::shared_ptr<vk::raii::Device> &device, const Settings &settings);
        ~GraphicsPipeline() = default;

        inline const vk::raii::Pipeline &get() const noexcept { return m_pipeline; }

        inline const vk::raii::Pipeline &operator*() const noexcept { return m_pipeline; }

        inline const vk::raii::Pipeline *operator->() const noexcept { return &m_pipeline; }

      private:
        std::shared_ptr<vk::raii::Device>          m_device;
        std::vector<std::shared_ptr<ShaderModule>> m_shader_modules;
        std::shared_ptr<PipelineLayout>            m_layout;
        std::shared_ptr<vk::raii::RenderPass>      m_render_pass;
        vk::raii::Pipeline                         m_pipeline = nullptr;
    };

} // namespace neuron::render
