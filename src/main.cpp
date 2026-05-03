#include "config.hpp"
#include "request.hpp"
#include "router.hpp"
#include "server.hpp"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string_view>
#include <thread>
#include <vector>

auto main(int argc, char** argv) -> int {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::vector<std::string_view> args{argv + 1, argv + argc};
    auto config_result = tinyhttp::parse_args(args);
    if (!config_result) {
        std::cerr << "Failed to parse arguments: " << config_result.error() << "\n";
        return 1;
    }

    tinyhttp::Router router{config_result->directory};
    tinyhttp::Server server{"0.0.0.0", 4221};

    if (auto result = server.listen(); !result) {
        std::cerr << "Failed to listen: " << result.error().message() << "\n";
        return 1;
    }

    auto handle_connection = [&router](tinyhttp::Connection conn) {
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
        } else {
            resp = router.dispatch(*parse_result);
        }

        auto data = resp.serialize();
        if (auto result = conn.send(data); !result) {
            std::cerr << "Failed to send: " << result.error().message() << "\n";
        }
    };

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
