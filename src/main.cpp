#include <spdlog/spdlog.h>

#include "neuron/engine.hpp"
#include "neuron/render/buffer.hpp"
#include "neuron/render/command_pool.hpp"
#include "neuron/render/graphics_pipeline.hpp"
#include "neuron/render/pipeline_layout.hpp"
#include "neuron/window.hpp"


#include <iostream>

struct PushConstants {
    glm::vec2 position;
    glm::vec2 scale;
    glm::vec4 color;
};

struct UniformData {
    glm::mat4 projectionView;
};

void run([[maybe_unused]] const std::shared_ptr<neuron::Engine> &engine) {
    const auto window = std::make_shared<neuron::Window>(neuron::Window::Attributes{
        .size  = {800, 600},
        .title = "Hello!",
    });

    const auto render_context = std::make_shared<neuron::render::RenderContext>(window);

    const auto command_pool   = std::make_unique<neuron::render::CommandPool>(render_context->device(), render_context->queue_family().value(), true);
    const auto command_buffer = command_pool->allocate_one();

    const auto pipeline_layout = std::make_shared<neuron::render::PipelineLayout>(
        render_context->device(),
        neuron::render::PipelineLayout::Settings{
            .push_constant_ranges = {
                vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants))}});

    const auto vsh = render_context->load_shader("shaders/bin/vert.glsl.spv");
    const auto fsh = render_context->load_shader("shaders/bin/frag.glsl.spv");

    std::vector<glm::vec2> vertices = {
        {0.5f, 0.5f},
        {-0.5f, 0.5f},
        {0.5f, -0.5f},
        {-0.5f, -0.5f}
    };

    const auto buffer = neuron::render::Buffer::create(
        render_context->device(), render_context->allocator(), vertices, vk::BufferUsageFlagBits::eVertexBuffer, true);

    const auto graphics_pipeline = std::make_unique<neuron::render::GraphicsPipeline>(
        render_context->device(),
        neuron::render::GraphicsPipeline::Settings{
            .vertex_bindings   = {vk::VertexInputBindingDescription(0, sizeof(glm::vec2))},
            .vertex_attributes = {vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat)},
            .dynamic_states    = {vk::DynamicState::eScissor, vk::DynamicState::eViewport, vk::DynamicState::ePrimitiveTopology},
            .shaders =
                {
                    {.module = vsh, .stage = vk::ShaderStageFlagBits::eVertex},
                    {.module = fsh, .stage = vk::ShaderStageFlagBits::eFragment},
                },
            .color_attachments =
                {
                    {true, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd, vk::BlendFactor::eOne,
                     vk::BlendFactor::eZero, vk::BlendOp::eAdd,
                     vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                         vk::ColorComponentFlagBits::eA},
                },
            .viewport = render_context->viewport_full(0.0f, 1.0f),
            .scissor  = render_context->scissor_full(),
            .layout   = pipeline_layout,

            .rendering_attachment_formats = neuron::render::RenderingAttachmentFormats{.color_attachments =
                                                                                           {render_context->surface_configuration()->image_format}},
        });


    while (!window->should_close()) {
        neuron::Window::poll();

        {
            auto frame_info = render_context->begin_frame();

            command_buffer.reset();
            command_buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
            {
                vk::ImageMemoryBarrier2 l1{};
                l1.image            = frame_info.image;
                l1.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
                l1.oldLayout        = vk::ImageLayout::eUndefined;
                l1.newLayout        = vk::ImageLayout::eColorAttachmentOptimal;
                l1.srcAccessMask    = vk::AccessFlagBits2::eNone;
                l1.dstAccessMask    = vk::AccessFlagBits2::eColorAttachmentWrite;
                l1.srcStageMask     = vk::PipelineStageFlagBits2::eTopOfPipe;
                l1.dstStageMask     = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
                command_buffer.pipelineBarrier2(vk::DependencyInfo({}, {}, {}, l1));
            }

            {
                vk::ClearColorValue clear_color{0.0f, 0.0f, 0.0f, 1.0f};

                vk::RenderingAttachmentInfo attachment{
                    frame_info.image_view,          vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ResolveModeFlagBits::eNone, {},
                    vk::ImageLayout::eUndefined,    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore,  clear_color,
                };

                command_buffer.beginRendering(
                    vk::RenderingInfo({}, vk::Rect2D({0, 0}, render_context->surface_configuration()->image_extent), 1, 0, attachment));
                
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **graphics_pipeline);
                command_buffer.setViewport(0, render_context->viewport_full(0.0, 1.0));
                command_buffer.setScissor(0, render_context->scissor_full());
                command_buffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleStrip);

                command_buffer.bindVertexBuffers(0, ***buffer, {0});

                PushConstants push_constant{
                    .position = {0.0, 0.0},
                    .scale = {1.8, 1.8},
                    .color = {1.0, 0.0, 0.0, 1.0}
                };

                command_buffer.pushConstants<PushConstants>(***pipeline_layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, push_constant);
                command_buffer.draw(4, 1, 0, 0);

                push_constant.position = {-0.5, 0.5};
                push_constant.scale = {-0.75, -0.75};
                push_constant.color = {0.0, 1.0, 0.0, 1.0};

                command_buffer.pushConstants<PushConstants>(***pipeline_layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, push_constant);
                command_buffer.draw(4, 1, 0, 0);

                push_constant.position = {0.5, -0.5};
                push_constant.scale = {0.75, 0.75};
                push_constant.color = {0.0, 0.0, 1.0, 1.0};

                command_buffer.pushConstants<PushConstants>(***pipeline_layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, push_constant);
                command_buffer.draw(4, 1, 0, 0);

                command_buffer.endRendering();
            }


            {
                vk::ImageMemoryBarrier2 l2{};
                l2.image            = frame_info.image;
                l2.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
                l2.oldLayout        = vk::ImageLayout::eColorAttachmentOptimal;
                l2.newLayout        = vk::ImageLayout::ePresentSrcKHR;
                l2.srcAccessMask    = vk::AccessFlagBits2::eColorAttachmentWrite;
                l2.dstAccessMask    = vk::AccessFlagBits2::eNone;
                l2.srcStageMask     = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
                l2.dstStageMask     = vk::PipelineStageFlagBits2::eBottomOfPipe;
                command_buffer.pipelineBarrier2(vk::DependencyInfo({}, {}, {}, l2));
            }
            command_buffer.end();

            {
                vk::CommandBufferSubmitInfo cbi{command_buffer};

                vk::SemaphoreSubmitInfo wait{*frame_info.image_available_semaphore, 0, vk::PipelineStageFlagBits2::eTopOfPipe, 0};
                vk::SemaphoreSubmitInfo signal{*frame_info.render_finished_semaphore, 0, vk::PipelineStageFlagBits2::eBottomOfPipe, 0};

                vk::SubmitInfo2 si{};
                si.setCommandBufferInfos(cbi);
                si.setWaitSemaphoreInfos(wait);
                si.setSignalSemaphoreInfos(signal);

                render_context->queue()->submit2(si, *frame_info.in_flight_fence);
            }

            render_context->end_frame(std::move(frame_info));
        }
    }

    render_context->device()->waitIdle();
}

int main() {
    try {
        const auto engine = std::make_shared<neuron::Engine>();
        run(engine);
    } catch (std::exception &e) {
        spdlog::critical("{}", e.what());
        spdlog::shutdown();
        return 1;
    }
    return 0;
}
