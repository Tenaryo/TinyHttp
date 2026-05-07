#include "response.hpp"

#include <cstring>
#include <format>
#include <iterator>

namespace tinyhttp {

auto Response::set_status(uint16_t code, std::string_view reason) -> Response& {
    status_code_ = code;
    reason_ = reason;
    return *this;
}

auto Response::add_header(std::string_view key, std::string_view value) -> Response& {
    headers_.emplace_back(key, value);
    return *this;
}

auto Response::set_body(std::span<const std::byte> body) -> Response& {
    body_.assign(body.begin(), body.end());
    return *this;
}

auto Response::set_body(std::vector<std::byte>&& body) -> Response& {
    body_ = std::move(body);
    return *this;
}

auto Response::serialize() const -> std::vector<std::byte> {
    auto raw = std::format("HTTP/1.1 {} {}\r\n", status_code_, reason_);
    for (const auto& [key, value] : headers_) {
        std::format_to(std::back_inserter(raw), "{}: {}\r\n", key, value);
    }
    raw += "\r\n";

    std::vector<std::byte> bytes;
    bytes.reserve(raw.size() + body_.size());
    auto p = reinterpret_cast<const std::byte*>(raw.data());
    bytes.insert(bytes.end(), p, p + raw.size());
    bytes.insert(bytes.end(), body_.begin(), body_.end());
    return bytes;
}

} // namespace tinyhttp
