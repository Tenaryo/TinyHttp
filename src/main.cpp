#include "response.hpp"
#include "server.hpp"

#include <cstdlib>
#include <iostream>

auto main() -> int {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    tinyhttp::Server server{"0.0.0.0", 4221};

    if (auto result = server.listen(); !result) {
        std::cerr << "Failed to listen: " << result.error().message() << "\n";
        return 1;
    }

    std::cout << "Waiting for a client to connect...\n";

    auto conn = server.accept();
    if (!conn) {
        std::cerr << "Failed to accept: " << conn.error().message() << "\n";
        return 1;
    }

    std::cout << "Client connected\n";

    tinyhttp::Response resp;
    auto data = resp.serialize();

    if (auto result = conn->send(data); !result) {
        std::cerr << "Failed to send: " << result.error().message() << "\n";
        return 1;
    }

    return 0;
}
