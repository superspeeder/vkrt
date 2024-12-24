#include "shader_module.hpp"

#include <fstream>

namespace neuron::render {
    ShaderModule::ShaderModule(const std::shared_ptr<vk::raii::Device> &device, std::vector<uint32_t> code) : m_device(device) {
        m_shader_module = vk::raii::ShaderModule(*m_device, vk::ShaderModuleCreateInfo({}, code));
    }

    std::shared_ptr<ShaderModule> ShaderModule::load(std::shared_ptr<vk::raii::Device> device, const std::filesystem::path &path) {
        std::ifstream f(path, std::ios::binary | std::ios::ate);

        if (!f.is_open()) {
            throw std::runtime_error("failed to open " + path.string());
        }

        const std::streampos end  = f.tellg();
        const std::size_t    size = end / sizeof(uint32_t);

        std::vector<uint32_t> code(size);
        f.seekg(0);
        f.read(reinterpret_cast<char *>(code.data()), size * sizeof(uint32_t));
        f.close();
        return std::make_shared<ShaderModule>(device, code);
    }
} // namespace neuron::render
