#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include <gtest/gtest.h>

#include "request.hpp"
#include "response.hpp"
#include "router.hpp"
#include "server.hpp"

static constexpr uint16_t TEST_PORT = 42201;

auto connect_and_read(uint16_t port, std::string_view request) -> std::string {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_GE(fd, 0) << "failed to create client socket";

    struct sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);

    int rc = ::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    EXPECT_EQ(rc, 0) << "failed to connect to server";

    if (!request.empty()) {
        auto sent = ::send(fd, request.data(), request.size(), 0);
        EXPECT_EQ(sent, static_cast<ssize_t>(request.size())) << "failed to send request";
    }

    char buf[4096]{};
    auto n = ::recv(fd, buf, sizeof(buf), 0);
    EXPECT_GT(n, 0) << "failed to recv response";

    ::close(fd);
    return {buf, static_cast<size_t>(n)};
}

auto route_response(std::string_view raw,
                    const std::filesystem::path& dir = {}) -> std::vector<std::byte> {
    auto parse_result = tinyhttp::parse_request(raw);
    if (!parse_result) {
        tinyhttp::Response resp;
        resp.set_status(400, "Bad Request");
        return resp.serialize();
    }

    tinyhttp::Router router{dir};
    return router.dispatch(*parse_result).serialize();
}

TEST(HttpResponse, Http200Root) {
    tinyhttp::Server server{"0.0.0.0", TEST_PORT};
    auto listen_result = server.listen();
    ASSERT_TRUE(listen_result.has_value()) << "server listen failed";

    auto accept_future = std::async(std::launch::async, [&] {
        auto conn_result = server.accept();
        ASSERT_TRUE(conn_result.has_value()) << "accept failed";

        char buf[4096]{};
        auto recv_result = conn_result->recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
        ASSERT_TRUE(recv_result.has_value()) << "recv failed";

        auto raw = std::string_view{buf, *recv_result};
        auto data = route_response(raw);
        auto send_result = conn_result->send(data);
        ASSERT_TRUE(send_result.has_value()) << "send failed";
    });

    auto response = connect_and_read(TEST_PORT, "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    accept_future.wait();

    EXPECT_EQ(response, "HTTP/1.1 200 OK\r\n\r\n");
}

TEST(HttpResponse, Http404NotFound) {
    constexpr uint16_t port = TEST_PORT + 1;
    tinyhttp::Server server{"0.0.0.0", port};
    auto listen_result = server.listen();
    ASSERT_TRUE(listen_result.has_value()) << "server listen failed";

    auto accept_future = std::async(std::launch::async, [&] {
        auto conn_result = server.accept();
        ASSERT_TRUE(conn_result.has_value()) << "accept failed";

        char buf[4096]{};
        auto recv_result = conn_result->recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
        ASSERT_TRUE(recv_result.has_value()) << "recv failed";

        auto raw = std::string_view{buf, *recv_result};
        auto data = route_response(raw);
        auto send_result = conn_result->send(data);
        ASSERT_TRUE(send_result.has_value()) << "send failed";
    });

    auto response = connect_and_read(port, "GET /abcdefg HTTP/1.1\r\nHost: localhost\r\n\r\n");
    accept_future.wait();

    EXPECT_EQ(response, "HTTP/1.1 404 Not Found\r\n\r\n");
}

TEST(HttpResponse, EchoEndpoint) {
    constexpr uint16_t port = TEST_PORT + 2;
    tinyhttp::Server server{"0.0.0.0", port};
    auto listen_result = server.listen();
    ASSERT_TRUE(listen_result.has_value()) << "server listen failed";

    auto accept_future = std::async(std::launch::async, [&] {
        auto conn_result = server.accept();
        ASSERT_TRUE(conn_result.has_value()) << "accept failed";

        char buf[4096]{};
        auto recv_result = conn_result->recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
        ASSERT_TRUE(recv_result.has_value()) << "recv failed";

        auto raw = std::string_view{buf, *recv_result};
        auto data = route_response(raw);
        auto send_result = conn_result->send(data);
        ASSERT_TRUE(send_result.has_value()) << "send failed";
    });

    auto response = connect_and_read(port, "GET /echo/abc HTTP/1.1\r\nHost: localhost\r\n\r\n");
    accept_future.wait();

    EXPECT_EQ(response,
              "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\nabc");
}

TEST(HttpResponse, UserAgentEndpoint) {
    constexpr uint16_t port = TEST_PORT + 3;
    tinyhttp::Server server{"0.0.0.0", port};
    auto listen_result = server.listen();
    ASSERT_TRUE(listen_result.has_value()) << "server listen failed";

    auto accept_future = std::async(std::launch::async, [&] {
        auto conn_result = server.accept();
        ASSERT_TRUE(conn_result.has_value()) << "accept failed";

        char buf[4096]{};
        auto recv_result = conn_result->recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
        ASSERT_TRUE(recv_result.has_value()) << "recv failed";

        auto raw = std::string_view{buf, *recv_result};
        auto data = route_response(raw);
        auto send_result = conn_result->send(data);
        ASSERT_TRUE(send_result.has_value()) << "send failed";
    });

    auto response = connect_and_read(
        port, "GET /user-agent HTTP/1.1\r\nHost: localhost\r\nUser-Agent: foobar/1.2.3\r\n\r\n");
    accept_future.wait();

    EXPECT_EQ(response,
              "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nfoobar/"
              "1.2.3");
}

static void handle_one(tinyhttp::Connection conn) {
    char buf[4096]{};
    auto recv_result = conn.recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
    if (!recv_result)
        return;
    auto raw = std::string_view{buf, *recv_result};
    auto data = route_response(raw);
    conn.send(data);
}

TEST(ConcurrentConnections, MultipleClientsGetResponse) {
    constexpr uint16_t port = TEST_PORT + 10;
    constexpr int num_clients = 5;

    tinyhttp::Server server{"0.0.0.0", port};
    auto listen_result = server.listen();
    ASSERT_TRUE(listen_result.has_value()) << "server listen failed";

    auto server_done = std::async(std::launch::async, [&] {
        for (int i = 0; i < num_clients; ++i) {
            auto conn_result = server.accept();
            ASSERT_TRUE(conn_result.has_value()) << "accept failed";
            std::thread(handle_one, std::move(*conn_result)).detach();
        }
    });

    std::vector<std::future<std::string>> client_futures;
    for (int i = 0; i < num_clients; ++i) {
        client_futures.push_back(std::async(std::launch::async, [=] {
            return connect_and_read(port, "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
        }));
    }

    for (auto& f : client_futures) {
        EXPECT_EQ(f.get(), "HTTP/1.1 200 OK\r\n\r\n");
    }

    server_done.wait();
}

TEST(HttpResponse, FilesEndpoint200) {
    const auto test_dir = std::filesystem::temp_directory_path() / "tinyhttp_test_files";
    std::filesystem::create_directories(test_dir);
    {
        std::ofstream ofs(test_dir / "foo");
        ofs << "Hello, World!";
    }

    constexpr uint16_t port = TEST_PORT + 4;
    tinyhttp::Server server{"0.0.0.0", port};
    auto listen_result = server.listen();
    ASSERT_TRUE(listen_result.has_value()) << "server listen failed";

    auto accept_future = std::async(std::launch::async, [&] {
        auto conn_result = server.accept();
        ASSERT_TRUE(conn_result.has_value()) << "accept failed";

        char buf[4096]{};
        auto recv_result = conn_result->recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
        ASSERT_TRUE(recv_result.has_value()) << "recv failed";

        auto raw = std::string_view{buf, *recv_result};
        auto data = route_response(raw, test_dir);
        auto send_result = conn_result->send(data);
        ASSERT_TRUE(send_result.has_value()) << "send failed";
    });

    auto response = connect_and_read(port, "GET /files/foo HTTP/1.1\r\nHost: localhost\r\n\r\n");
    accept_future.wait();

    EXPECT_EQ(response,
              "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: "
              "13\r\n\r\nHello, World!");

    std::filesystem::remove_all(test_dir);
}

TEST(HttpResponse, FilesEndpoint404) {
    const auto test_dir = std::filesystem::temp_directory_path() / "tinyhttp_test_files_404";
    std::filesystem::create_directories(test_dir);

    constexpr uint16_t port = TEST_PORT + 5;
    tinyhttp::Server server{"0.0.0.0", port};
    auto listen_result = server.listen();
    ASSERT_TRUE(listen_result.has_value()) << "server listen failed";

    auto accept_future = std::async(std::launch::async, [&] {
        auto conn_result = server.accept();
        ASSERT_TRUE(conn_result.has_value()) << "accept failed";

        char buf[4096]{};
        auto recv_result = conn_result->recv({reinterpret_cast<std::byte*>(buf), sizeof(buf)});
        ASSERT_TRUE(recv_result.has_value()) << "recv failed";

        auto raw = std::string_view{buf, *recv_result};
        auto data = route_response(raw, test_dir);
        auto send_result = conn_result->send(data);
        ASSERT_TRUE(send_result.has_value()) << "send failed";
    });

    auto response =
        connect_and_read(port, "GET /files/non_existent_file HTTP/1.1\r\nHost: localhost\r\n\r\n");
    accept_future.wait();

    EXPECT_EQ(response, "HTTP/1.1 404 Not Found\r\n\r\n");

    std::filesystem::remove_all(test_dir);
}
