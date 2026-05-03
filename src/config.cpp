#include "config.hpp"

#include <format>

namespace tinyhttp {

auto parse_args(std::span<const std::string_view> args)
    -> std::expected<ServerConfig, std::string> {
    ServerConfig config;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--directory") {
            if (i + 1 >= args.size()) {
                return std::unexpected(std::format("missing value for --directory"));
            }
            config.directory = args[++i];
        } else if (args[i].starts_with("--")) {
            return std::unexpected(std::format("unknown option: {}", args[i]));
        }
    }
    return config;
}

} // namespace tinyhttp
