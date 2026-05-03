#include "request.hpp"
#include "response.hpp"
#include "server.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

using tinyhttp::Connection;

static void handle_connection(Connection conn) {
    std::byte buf[4096];
    auto recv_result = conn.recv(buf);
    if (!recv_result) {
        std::cerr << "Failed to recv: " << recv_result.error().message() << "\n";
        return;
    }

    auto raw = std::string_view{reinterpret_cast<const char*>(buf), *recv_result};
    auto parse_result = tinyhttp::parse_request(raw);

    tinyhttp::Response resp;
    if (!parse_result) {
        resp.set_status(400, "Bad Request");
    } else if (parse_result->path == "/") {
        resp.set_status(200, "OK");
    } else if (auto echo = tinyhttp::match_echo_path(parse_result->path)) {
        resp.set_status(200, "OK");
        resp.add_header("Content-Type", "text/plain");
        auto body = std::string(*echo);
        resp.add_header("Content-Length", std::to_string(body.size()));
        resp.set_body({reinterpret_cast<const std::byte*>(body.data()), body.size()});
    } else if (parse_result->path == "/user-agent") {
        resp.set_status(200, "OK");
        resp.add_header("Content-Type", "text/plain");
        auto ua = std::string(parse_result->get_header("User-Agent").value_or(""));
        resp.add_header("Content-Length", std::to_string(ua.size()));
        resp.set_body({reinterpret_cast<const std::byte*>(ua.data()), ua.size()});
    } else {
        resp.set_status(404, "Not Found");
    }

    auto data = resp.serialize();
    if (auto result = conn.send(data); !result) {
        std::cerr << "Failed to send: " << result.error().message() << "\n";
    }
}

auto main() -> int {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    tinyhttp::Server server{"0.0.0.0", 4221};

    if (auto result = server.listen(); !result) {
        std::cerr << "Failed to listen: " << result.error().message() << "\n";
        return 1;
    }

    while (true) {
        std::cout << "Waiting for a client to connect...\n";

        auto conn_result = server.accept();
        if (!conn_result) {
            std::cerr << "Failed to accept: " << conn_result.error().message() << "\n";
            continue;
        }

        std::jthread{handle_connection, std::move(*conn_result)}.detach();
    }

    return 0;
}
