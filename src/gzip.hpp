#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace tinyhttp {

auto compress_gzip(std::string_view input) -> std::vector<std::byte>;

} // namespace tinyhttp
