#pragma once

#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace tinyhttp {

auto compress_gzip(std::string_view input) -> std::expected<std::vector<std::byte>, std::string>;

} // namespace tinyhttp
