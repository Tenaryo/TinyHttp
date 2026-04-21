#include "server.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace tinyhttp {

Server::Server(std::string_view /*host*/, uint16_t port) : port_{port} {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
}

Server::~Server() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

auto Server::listen() -> std::expected<void, std::error_code> {
    int reuse = 1;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        return std::unexpected(std::error_code{errno, std::system_category()});
    }

    struct sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = ::htons(port_);

    if (::bind(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
        return std::unexpected(std::error_code{errno, std::system_category()});
    }

    if (::listen(fd_, 5) != 0) {
        return std::unexpected(std::error_code{errno, std::system_category()});
    }

    return {};
}

auto Server::accept() -> std::expected<Connection, std::error_code> {
    struct sockaddr_in client_addr {};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = ::accept(fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
    if (client_fd < 0) {
        return std::unexpected(std::error_code{errno, std::system_category()});
    }
    return Connection{client_fd};
}

} // namespace tinyhttp
