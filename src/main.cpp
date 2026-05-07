#include "config.hpp"
#include "connection_handler.hpp"
#include "router.hpp"
#include "server.hpp"
#include "thread_pool.hpp"

#include <cstdlib>
#include <iostream>
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
    tinyhttp::Server server{4221};
    tinyhttp::ThreadPool pool(std::thread::hardware_concurrency());

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

        pool.enqueue([conn = std::move(*conn_result), &router]() mutable {
            tinyhttp::handle_connection(std::move(conn), router);
        });
    }

    return 0;
}
