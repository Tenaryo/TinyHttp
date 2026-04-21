#include <cassert>
#include <cstring>
#include <future>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

#include "request.hpp"
#include "response.hpp"
#include "server.hpp"

static constexpr uint16_t TEST_PORT = 4221;

auto connect_and_read(uint16_t port, std::string_view request) -> std::string {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    assert(fd >= 0 && "failed to create client socket");

    struct sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);

    int rc = ::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    assert(rc == 0 && "failed to connect to server");

    if (!request.empty()) {
        auto sent = ::send(fd, request.data(), request.size(), 0);
        assert(sent == static_cast<ssize_t>(request.size()) && "failed to send request");
    }

    char buf[4096]{};
    auto n = ::recv(fd, buf, sizeof(buf), 0);
    assert(n > 0 && "failed to recv response");

    ::close(fd);
    return {buf, static_cast<size_t>(n)};
}

auto route_response(std::string_view raw) -> std::vector<std::byte> {
    auto parse_result = tinyhttp::parse_request(raw);
    tinyhttp::Response resp;
    if (parse_result && parse_result->path == "/") {
        resp.set_status(200, "OK");
    } else {
        resp.set_status(404, "Not Found");
    }
    return resp.serialize();
}

auto test_http_200_root() -> void {
    tinyhttp::Server server{"0.0.0.0", TEST_PORT};
    auto listen_result = server.listen();
    assert(listen_result.has_value() && "server listen failed");

    auto accept_future = std::async(std::launch::async, [&] {
        auto conn_result = server.accept();
        assert(conn_result.has_value() && "accept failed");

        char buf[4096]{};
        auto recv_result = conn_result->recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
        assert(recv_result.has_value() && "recv failed");

        auto raw = std::string_view{buf, *recv_result};
        auto data = route_response(raw);
        auto send_result = conn_result->send(data);
        assert(send_result.has_value() && "send failed");
    });

    auto response = connect_and_read(TEST_PORT, "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    accept_future.wait();

    assert(response == "HTTP/1.1 200 OK\r\n\r\n" && "unexpected response");

    std::cout << "test_http_200_root: PASSED\n";
}

auto test_http_404_not_found() -> void {
    tinyhttp::Server server{"0.0.0.0", TEST_PORT};
    auto listen_result = server.listen();
    assert(listen_result.has_value() && "server listen failed");

    auto accept_future = std::async(std::launch::async, [&] {
        auto conn_result = server.accept();
        assert(conn_result.has_value() && "accept failed");

        char buf[4096]{};
        auto recv_result = conn_result->recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
        assert(recv_result.has_value() && "recv failed");

        auto raw = std::string_view{buf, *recv_result};
        auto data = route_response(raw);
        auto send_result = conn_result->send(data);
        assert(send_result.has_value() && "send failed");
    });

    auto response = connect_and_read(TEST_PORT, "GET /abcdefg HTTP/1.1\r\nHost: localhost\r\n\r\n");
    accept_future.wait();

    assert(response == "HTTP/1.1 404 Not Found\r\n\r\n" && "unexpected response");

    std::cout << "test_http_404_not_found: PASSED\n";
}

auto main() -> int {
    test_http_200_root();
    test_http_404_not_found();
    std::cout << "All tests passed!\n";
    return 0;
}
