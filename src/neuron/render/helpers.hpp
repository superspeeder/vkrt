#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "neuron/render/pipeline_layout.hpp"

namespace neuron::render {
    struct ImageLayoutState {
        vk::ImageLayout         layout;
        vk::AccessFlags2        access;
        vk::PipelineStageFlags2 stage_mask;
        uint32_t                owning_queue_family = 0;
    };

    class CommandRecorder {
        bool m_started = false;

      public:
        CommandRecorder(const CommandRecorder &other)                = delete;
        CommandRecorder(CommandRecorder &&other) noexcept            = delete;
        CommandRecorder &operator=(const CommandRecorder &other)     = delete;
        CommandRecorder &operator=(CommandRecorder &&other) noexcept = delete;

        explicit CommandRecorder(const vk::raii::CommandBuffer &command_buffer, std::optional<vk::CommandBufferBeginInfo> cbbi = std::nullopt)
            : m_command_buffer(command_buffer) {
            if (cbbi.has_value()) {
                m_command_buffer.reset();
                m_command_buffer.begin(cbbi.value());
                m_started = true;
            }
        }

        static inline CommandRecorder one_time_submit(const vk::raii::CommandBuffer &command_buffer) {
            return CommandRecorder(command_buffer, vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        }

        ~CommandRecorder() {
            if (m_started) {
                m_command_buffer.end();
            }
        }

        void image_transition(vk::Image image, const vk::ImageSubresourceRange &image_subresource_range, const ImageLayoutState &src,
                              const ImageLayoutState &dst) const;
        void image_transition(vk::Image image, const ImageLayoutState &src, const ImageLayoutState &dst) const;

        inline const vk::raii::CommandBuffer &operator*() const { return m_command_buffer; }

        inline const vk::raii::CommandBuffer *operator->() const { return &m_command_buffer; }


      private:
        const vk::raii::CommandBuffer &m_command_buffer;
    };

    class DynamicRenderingContext {
    public:
        DynamicRenderingContext(const CommandRecorder& recorder, const vk::RenderingInfo &rendering_info) : m_recorder(recorder) {
            m_recorder->beginRendering(rendering_info);
        };

        ~DynamicRenderingContext() {
            m_recorder->endRendering();
        };

        template<typename T>
        void push_constants(const std::shared_ptr<PipelineLayout> &pipeline_layout, vk::ShaderStageFlags stages, const T& data, std::ptrdiff_t offset = 0) const {
            m_recorder->pushConstants<T>(***pipeline_layout, stages, static_cast<uint32_t>(offset), value)
        };

    private:

        const CommandRecorder& m_recorder;
    };
} // namespace neuron::render
