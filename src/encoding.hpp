#pragma once

#include "request.hpp"

#include <optional>
#include <string_view>

namespace tinyhttp {

inline auto negotiate_encoding(const Request& req) -> std::optional<std::string_view> {
    auto header = req.get_header("Accept-Encoding");
    if (!header)
        return std::nullopt;

    auto v = *header;
    std::size_t pos = 0;
    while (pos < v.size()) {
        auto comma = v.find(',', pos);
        auto token = v.substr(pos, comma - pos);
        auto start = token.find_first_not_of(' ');
        if (start != std::string_view::npos) {
            auto end = token.find_last_not_of(' ');
            if (token.substr(start, end - start + 1) == "gzip")
                return "gzip";
        }
        if (comma == std::string_view::npos)
            break;
        pos = comma + 1;
    }
    return std::nullopt;
}

} // namespace tinyhttp
