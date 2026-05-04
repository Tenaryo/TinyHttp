#include "gzip.hpp"

namespace tinyhttp {

auto compress_gzip(std::string_view input) -> std::vector<std::byte> {
    return {reinterpret_cast<const std::byte*>(input.data()),
            reinterpret_cast<const std::byte*>(input.data() + input.size())};
}

} // namespace tinyhttp
