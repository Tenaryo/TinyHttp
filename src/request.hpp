#pragma once

#include <expected>
#include <string>
#include <string_view>

namespace tinyhttp {

struct Request {
    std::string_view method;
    std::string_view path;
    std::string_view version;
};

auto parse_request(std::string_view raw) -> std::expected<Request, std::string>;

} // namespace tinyhttp
