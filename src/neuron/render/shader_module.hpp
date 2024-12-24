#pragma once

#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

namespace neuron::render {

    class ShaderModule final {
      public:
        ShaderModule(const std::shared_ptr<vk::raii::Device> &device, std::vector<uint32_t> code);
        ~ShaderModule() = default;

        ShaderModule(const ShaderModule &other)                = delete;
        ShaderModule(ShaderModule &&other) noexcept            = default;
        ShaderModule &operator=(const ShaderModule &other)     = delete;
        ShaderModule &operator=(ShaderModule &&other) noexcept = default;

        static std::shared_ptr<ShaderModule> load(std::shared_ptr<vk::raii::Device> device, const std::filesystem::path &path);

        inline const vk::raii::ShaderModule &get() const noexcept { return m_shader_module; }

        inline const vk::raii::ShaderModule &operator*() const noexcept { return m_shader_module; }

        inline const vk::raii::ShaderModule *operator->() const noexcept { return &m_shader_module; }

      private:
        std::shared_ptr<vk::raii::Device> m_device;
        vk::raii::ShaderModule            m_shader_module = nullptr;
    };

} // namespace neuron::render
