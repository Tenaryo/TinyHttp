#include "response.hpp"

#include <cstring>
#include <format>

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

auto Response::serialize() const -> std::vector<std::byte> {
    auto raw = std::format("HTTP/1.1 {} {}\r\n", status_code_, reason_);
    for (const auto& [key, value] : headers_) {
        raw += std::format("{}: {}\r\n", key, value);
    }
    raw += "\r\n";
    raw.append(reinterpret_cast<const char*>(body_.data()), body_.size());

    auto bytes = std::vector<std::byte>(raw.size());
    std::memcpy(bytes.data(), raw.data(), raw.size());
    return bytes;
}

} // namespace tinyhttp
