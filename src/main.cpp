#include "config.hpp"
#include "connection_handler.hpp"
#include "router.hpp"
#include "server.hpp"

#include <cstdlib>
#include <functional>
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

    while (true) {
        std::cout << "Waiting for a client to connect...\n";

        auto conn_result = server.accept();
        if (!conn_result) {
            std::cerr << "Failed to accept: " << conn_result.error().message() << "\n";
            continue;
        }

        std::jthread{tinyhttp::handle_connection, std::move(*conn_result), std::cref(router)}
            .detach();
    }

    return 0;
}
