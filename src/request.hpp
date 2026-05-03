#pragma once

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace tinyhttp {

class Request {
  public:
    std::string_view method;
    std::string_view path;
    std::string_view version;
    std::string_view body;

    auto get_header(std::string_view name) const -> std::optional<std::string_view>;
  private:
    friend auto parse_request(std::string_view raw) -> std::expected<Request, std::string>;
    std::vector<std::pair<std::string_view, std::string_view>> headers_;
};

auto parse_request(std::string_view raw) -> std::expected<Request, std::string>;
auto match_echo_path(std::string_view path) -> std::optional<std::string_view>;

} // namespace tinyhttp
