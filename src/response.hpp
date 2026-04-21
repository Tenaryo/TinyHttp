#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace tinyhttp {

class Response {
  public:
    auto set_status(uint16_t code, std::string_view reason) -> Response&;
    auto add_header(std::string_view key, std::string_view value) -> Response&;
    auto set_body(std::span<const std::byte> body) -> Response&;
    auto serialize() const -> std::vector<std::byte>;
  private:
    uint16_t status_code_{200};
    std::string reason_{"OK"};
    std::vector<std::pair<std::string, std::string>> headers_;
    std::vector<std::byte> body_;
};

} // namespace tinyhttp
