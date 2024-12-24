#include "engine.hpp"

#include <spdlog/spdlog.h>

namespace neuron {
    Engine::Engine() {
#ifdef NDEBUG
        spdlog::set_level(spdlog::level::info);
#else
#ifdef NEURON_ENABLE_TRACE
        spdlog::set_level(spdlog::level::trace);
#else
        spdlog::set_level(spdlog::level::debug);
#endif
#endif
    }
} // namespace neuron
