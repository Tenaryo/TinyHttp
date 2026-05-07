#pragma once

#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace tinyhttp {

struct ServerConfig {
    std::filesystem::path directory;
};

[[nodiscard]] auto
parse_args(std::span<const std::string_view> args) -> std::expected<ServerConfig, std::string>;

} // namespace tinyhttp
