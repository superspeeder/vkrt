#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

#include <array>
#include <memory>
#include <vector>

namespace neuron::render {
    class Buffer final {
      public:
        Buffer(const std::shared_ptr<vk::raii::Device> &device, VmaAllocator allocator, std::size_t size, vk::BufferUsageFlags usage,
               bool host_mappable = true);
        Buffer(const std::shared_ptr<vk::raii::Device> &device, VmaAllocator allocator, const void *data, std::size_t size,
               vk::BufferUsageFlags usage, bool host_mappable = true);

        ~Buffer();

        template <typename T>
        static std::shared_ptr<Buffer> create(const std::shared_ptr<vk::raii::Device> &device, VmaAllocator allocator, const std::vector<T> &data,
                                              vk::BufferUsageFlags usage, bool host_mappable = true) {
            return std::make_shared<Buffer>(device, allocator, data.data(), data.size() * sizeof(T), usage, host_mappable);
        }

        template <typename T, std::size_t N>
        static std::shared_ptr<Buffer> create(const std::shared_ptr<vk::raii::Device> &device, VmaAllocator allocator, const std::array<T, N> &data,
                                              vk::BufferUsageFlags usage, bool host_mappable = true) {
            return std::make_shared<Buffer>(device, allocator, data.data(), N * sizeof(T), usage, host_mappable);
        }

        template <typename T>
        static std::shared_ptr<Buffer> create_obj(const std::shared_ptr<vk::raii::Device> &device, VmaAllocator allocator, const T &data,
                                                  vk::BufferUsageFlags usage, bool host_mappable = true) {
            return std::make_shared<Buffer>(device, allocator, &data, sizeof(T), usage, host_mappable);
        }

        inline const vk::raii::Buffer &get() const { return m_buffer; };

        inline const vk::raii::Buffer &operator*() const { return m_buffer; };

        inline const vk::raii::Buffer *operator->() const { return &m_buffer; };

        inline vk::DeviceSize size() const noexcept { return m_size; }

      private:
        std::shared_ptr<vk::raii::Device> m_device;
        VmaAllocator                      m_allocator;

        vk::raii::Buffer  m_buffer = nullptr;
        VmaAllocation     m_allocation;
        VmaAllocationInfo m_allocation_info;

        vk::DeviceSize m_size;
    };
} // namespace neuron::render
