#include "helpers.hpp"

namespace neuron::render {
    void CommandRecorder::image_transition(const vk::Image image, const vk::ImageSubresourceRange &image_subresource_range, const ImageLayoutState &src,
                                           const ImageLayoutState &dst) const {
        vk::ImageMemoryBarrier2 imb{};
        imb.image = image;
        imb.subresourceRange = image_subresource_range;
        imb.oldLayout = src.layout;
        imb.srcAccessMask = src.access;
        imb.srcStageMask = src.stage_mask;
        imb.srcQueueFamilyIndex = src.owning_queue_family;
        imb.newLayout = dst.layout;
        imb.dstAccessMask = dst.access;
        imb.dstStageMask = dst.stage_mask;
        imb.dstQueueFamilyIndex = dst.owning_queue_family;

        m_command_buffer.pipelineBarrier2(vk::DependencyInfo({}, {}, {}, imb));
    }

    void CommandRecorder::image_transition(const vk::Image image, const ImageLayoutState &src, const ImageLayoutState &dst) const {
        image_transition(image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1), src, dst);
    }
} // namespace neuron::render
