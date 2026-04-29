#include "request.hpp"

namespace tinyhttp {

auto parse_request(std::string_view raw) -> std::expected<Request, std::string> {
    auto line_end = raw.find("\r\n");
    if (line_end == std::string_view::npos) {
        return std::unexpected("invalid request: missing CRLF");
    }

    auto line = raw.substr(0, line_end);

    auto sp1 = line.find(' ');
    if (sp1 == std::string_view::npos) {
        return std::unexpected("invalid request: missing method-path separator");
    }

    auto sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string_view::npos) {
        return std::unexpected("invalid request: missing path-version separator");
    }

    return Request{
        .method = line.substr(0, sp1),
        .path = line.substr(sp1 + 1, sp2 - sp1 - 1),
        .version = line.substr(sp2 + 1),
    };
}

auto match_echo_path(std::string_view path) -> std::optional<std::string_view> {
    constexpr std::string_view prefix = "/echo/";
    if (path.starts_with(prefix))
        return path.substr(prefix.size());
    return std::nullopt;
}

} // namespace tinyhttp
