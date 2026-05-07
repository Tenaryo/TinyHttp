#pragma once

#include "request.hpp"

#include <optional>
#include <ranges>
#include <string_view>

namespace tinyhttp {

inline auto negotiate_encoding(const Request& req) -> std::optional<std::string_view> {
    auto header = req.get_header("Accept-Encoding");
    if (!header)
        return std::nullopt;

    auto trim = [](std::string_view sv) {
        auto start = sv.find_first_not_of(' ');
        if (start == std::string_view::npos)
            return std::string_view{};
        return sv.substr(start, sv.find_last_not_of(' ') - start + 1);
    };

    for (auto r : std::views::split(*header, ',')) {
        if (trim(std::string_view(r.begin(), r.end())) == "gzip")
            return "gzip";
    }
    return std::nullopt;
}

} // namespace tinyhttp
