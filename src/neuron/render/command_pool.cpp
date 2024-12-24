#include "command_pool.hpp"

namespace neuron::render {
    CommandPool::CommandPool(const std::shared_ptr<vk::raii::Device> &device, const uint32_t queue_family, const bool individually_resettable)
        : m_device(device), m_queue_family(queue_family), m_individually_resettable(individually_resettable) {
        m_command_pool = std::make_unique<vk::raii::CommandPool>(
            *m_device,
            vk::CommandPoolCreateInfo(individually_resettable ? vk::CommandPoolCreateFlagBits::eResetCommandBuffer : vk::CommandPoolCreateFlags{},
                                      queue_family));
    }

    vk::raii::CommandBuffers CommandPool::allocate(const std::size_t count, const vk::CommandBufferLevel level) const {
        return vk::raii::CommandBuffers(*m_device, vk::CommandBufferAllocateInfo(**m_command_pool, level, count));
    }

    vk::raii::CommandBuffer CommandPool::allocate_one(const vk::CommandBufferLevel level) const {
        return std::move(allocate(1, level)[0]);
    }

    void CommandPool::reset() const {
        m_command_pool->reset();
    }
} // namespace neuron::render
