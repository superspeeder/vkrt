#include "buffer.hpp"

namespace neuron::render {
    std::tuple<vk::Buffer, VmaAllocation, VmaAllocationInfo> alloc_buffer(const std::shared_ptr<vk::raii::Device> &device, VmaAllocator allocator,
                                                                          const void *data, std::size_t size, vk::BufferUsageFlags usage,
                                                                          bool host_mappable) {
        VkBuffer          buffer;
        VmaAllocation     alloc;
        VmaAllocationInfo ai;

        VmaAllocationCreateInfo aci{};
        vk::BufferCreateInfo    bci{};

        bci.size        = size;
        bci.usage       = usage;
        bci.sharingMode = vk::SharingMode::eExclusive;

        aci.usage = VMA_MEMORY_USAGE_AUTO;

        if (host_mappable) {
            aci.requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            aci.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                 VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        } else {
            aci.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            aci.usage          = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        }

        VkBufferCreateInfo bci_ = bci;

        if (vmaCreateBuffer(allocator, &bci_, &aci, &buffer, &alloc, &ai) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer");
        }

        if (host_mappable && data != nullptr) {
            void *map;
            vmaMapMemory(allocator, alloc, &map);
            std::memcpy(map, data, size);
            vmaUnmapMemory(allocator, alloc);
        } else if (data != nullptr) {
            throw std::logic_error("Staged copies not yet implemented");
        }

        return std::make_tuple(buffer, alloc, ai);
    }

    Buffer::Buffer(const std::shared_ptr<vk::raii::Device> &device, VmaAllocator allocator, std::size_t size, vk::BufferUsageFlags usage,
                   bool host_mappable)
        : m_device(device), m_allocator(allocator) {
        auto [buffer, alloc, ai] = alloc_buffer(device, allocator, nullptr, size, usage, host_mappable);

        m_buffer          = vk::raii::Buffer(*device, buffer);
        m_allocation      = alloc;
        m_allocation_info = ai;
        m_size = size;
    }

    Buffer::Buffer(const std::shared_ptr<vk::raii::Device> &device, VmaAllocator allocator, const void *data, std::size_t size,
                   vk::BufferUsageFlags usage, bool host_mappable)
        : m_device(device), m_allocator(allocator) {
        auto [buffer, alloc, ai] = alloc_buffer(device, allocator, data, size, usage, host_mappable);

        m_buffer          = vk::raii::Buffer(*device, buffer);
        m_allocation      = alloc;
        m_allocation_info = ai;
        m_size = size;
    }

    Buffer::~Buffer() {
        m_buffer = nullptr;
        vmaFreeMemory(m_allocator, m_allocation);
    }
} // namespace neuron::render
