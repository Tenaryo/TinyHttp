#include "request.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <ranges>

namespace tinyhttp {

auto Request::get_header(std::string_view name) const -> std::optional<std::string_view> {
    auto it = std::ranges::find_if(headers_, [&](const auto& pair) {
        return std::ranges::equal(pair.first, name, [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
        });
    });
    if (it != headers_.end())
        return it->second;
    return std::nullopt;
}

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

    Request req;
    req.method = line.substr(0, sp1);
    req.path = line.substr(sp1 + 1, sp2 - sp1 - 1);
    req.version = line.substr(sp2 + 1);

    auto pos = line_end + 2;
    while (pos < raw.size()) {
        auto next = raw.find("\r\n", pos);
        if (next == pos)
            break;
        if (next == std::string_view::npos)
            break;

        auto header_line = raw.substr(pos, next - pos);
        auto colon = header_line.find(':');
        if (colon != std::string_view::npos) {
            auto name = header_line.substr(0, colon);
            auto value = header_line.substr(colon + 1);
            auto value_start = value.find_first_not_of(' ');
            if (value_start != std::string_view::npos) {
                value = value.substr(value_start);
            }
            req.headers_.emplace_back(name, value);
        }
        pos = next + 2;
    }

    auto body_start = pos + 2;
    if (auto cl = req.get_header("Content-Length")) {
        size_t length = 0;
        auto [ptr, ec] = std::from_chars(cl->data(), cl->data() + cl->size(), length);
        if (ec == std::errc{} && length <= raw.size() - body_start) {
            req.body = raw.substr(body_start, length);
        }
    }

    return req;
}

auto match_echo_path(std::string_view path) -> std::optional<std::string_view> {
    constexpr std::string_view prefix = "/echo/";
    if (path.starts_with(prefix))
        return path.substr(prefix.size());
    return std::nullopt;
}

} // namespace tinyhttp
