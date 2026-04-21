#include "request.hpp"
#include "response.hpp"
#include "server.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>

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

        auto& conn = *conn_result;

        std::byte buf[4096];
        auto recv_result = conn.recv(buf);
        if (!recv_result) {
            std::cerr << "Failed to recv: " << recv_result.error().message() << "\n";
            continue;
        }

        auto raw = std::string_view{reinterpret_cast<const char*>(buf), *recv_result};
        auto parse_result = tinyhttp::parse_request(raw);

        tinyhttp::Response resp;
        if (parse_result && parse_result->path == "/") {
            resp.set_status(200, "OK");
        } else {
            resp.set_status(404, "Not Found");
        }

        auto data = resp.serialize();
        if (auto result = conn.send(data); !result) {
            std::cerr << "Failed to send: " << result.error().message() << "\n";
        }
    }

    return 0;
}
