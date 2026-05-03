#include "config.hpp"

namespace tinyhttp {

auto parse_args(std::span<const std::string_view>) -> std::expected<ServerConfig, std::string> {
    return std::unexpected("not implemented");
}

} // namespace tinyhttp
