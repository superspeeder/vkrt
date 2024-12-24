#pragma once

#include <memory>
#include <vulkan/vulkan_raii.hpp>

namespace neuron::render {

    class CommandPool final {
      public:
        CommandPool(const std::shared_ptr<vk::raii::Device> &device, uint32_t queue_family, bool individually_resettable = true);
        ~CommandPool() = default;

        CommandPool(const CommandPool &other)                = delete;
        CommandPool(CommandPool &&other) noexcept            = default;
        CommandPool &operator=(const CommandPool &other)     = delete;
        CommandPool &operator=(CommandPool &&other) noexcept = default;

        inline const vk::raii::CommandPool &get() const { return *m_command_pool; };

        inline const vk::raii::CommandPool &operator*() const { return *m_command_pool; };

        inline const vk::raii::CommandPool *operator->() const { return m_command_pool.get(); };

        vk::raii::CommandBuffers allocate(std::size_t count, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;
        vk::raii::CommandBuffer  allocate_one(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;

        void reset() const;

      private:
        std::shared_ptr<vk::raii::Device>      m_device;
        uint32_t                               m_queue_family;
        bool                                   m_individually_resettable;
        std::unique_ptr<vk::raii::CommandPool> m_command_pool;
    };

} // namespace neuron::render
