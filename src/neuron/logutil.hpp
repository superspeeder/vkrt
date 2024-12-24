#pragma once

#include <spdlog/spdlog.h>
#include <source_location>

namespace neuron::logutil {
    struct tracetool {
        std::source_location loc;

        inline explicit tracetool(const std::source_location &loc = std::source_location::current()) : loc(loc) {
            spdlog::trace("{}: {}: Enter", loc.file_name(), loc.function_name());
        }

        inline ~tracetool() {
            spdlog::trace("{}: {}: Exit", loc.file_name(), loc.function_name());
        }
    };
}

#ifdef NEURON_ENABLE_TRACE
#define NEURON_TRACE_TOOL() ::neuron::logutil::tracetool _neurontracetool(std::source_location::current())
#else
#define NEURON_TRACE_TOOL()
#endif