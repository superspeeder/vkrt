#include "neuron/engine.hpp"
#include "neuron/render/command_pool.hpp"
#include "neuron/window.hpp"


#include <iostream>

void run([[maybe_unused]] const std::shared_ptr<neuron::Engine> &engine) {
    const auto window = std::make_shared<neuron::Window>(neuron::Window::Attributes{
        .size  = {800, 600},
        .title = "Hello!",
    });

    const auto render_context = std::make_shared<neuron::render::RenderContext>(window);

    const auto command_pool   = std::make_unique<neuron::render::CommandPool>(render_context->device(), render_context->queue_family().value(), true);
    const auto command_buffer = command_pool->allocate_one();

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
                vk::ClearColorValue clear_color{1.0f, 0.0f, 1.0f, 1.0f};

                vk::RenderingAttachmentInfo attachment{
                    frame_info.image_view,          vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ResolveModeFlagBits::eNone, {},
                    vk::ImageLayout::eUndefined,    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore,  clear_color,
                };

                command_buffer.beginRendering(
                    vk::RenderingInfo({}, vk::Rect2D({0, 0}, render_context->surface_configuration()->image_extent), 1, 0, attachment));
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
}

int main() {
    const auto engine = std::make_shared<neuron::Engine>();
    run(engine);
    return 0;
}
