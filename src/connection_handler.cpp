#include "connection_handler.hpp"
#include "request.hpp"
#include "response.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <ranges>
#include <string_view>

namespace tinyhttp {

void handle_connection(Connection conn, const Router& router) {
    std::array<std::byte, 4096> buf{};

    while (true) {
        auto recv_result = conn.recv(buf);
        if (!recv_result) {
            std::cerr << "Failed to recv: " << recv_result.error().message() << "\n";
            return;
        }
        if (*recv_result == 0)
            return;

        auto raw = std::string_view{reinterpret_cast<const char*>(buf.data()), *recv_result};
        auto parse_result = parse_request(raw);

        Response resp;
        if (!parse_result) {
            resp.set_status(400, "Bad Request");
            conn.send(resp.serialize());
            return;
        }

        resp = router.dispatch(*parse_result);

        bool should_close = false;
        if (auto conn_hdr = parse_result->get_header("Connection")) {
            using namespace std::string_view_literals;
            constexpr auto close = "close"sv;
            should_close = std::ranges::equal(*conn_hdr, close, [](char a, char b) {
                return std::tolower(static_cast<unsigned char>(a)) ==
                       std::tolower(static_cast<unsigned char>(b));
            });
        }

        if (should_close) {
            resp.add_header("Connection", "close");
        }

        if (auto result = conn.send(resp.serialize()); !result) {
            std::cerr << "Failed to send: " << result.error().message() << "\n";
            return;
        }

        if (should_close)
            return;
    }
}

} // namespace tinyhttp
