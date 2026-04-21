#include "connection.hpp"

#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

namespace tinyhttp {

Connection::Connection(int fd) noexcept : fd_{fd} {}

Connection::Connection(Connection&& other) noexcept : fd_{other.fd_} { other.fd_ = -1; }

auto Connection::operator=(Connection&& other) noexcept -> Connection& {
    if (this != &other) {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

Connection::~Connection() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

auto Connection::send(std::span<const std::byte> data) -> std::expected<void, std::error_code> {
    while (!data.empty()) {
        auto sent = ::send(fd_, data.data(), data.size(), 0);
        if (sent < 0) {
            return std::unexpected(std::error_code{errno, std::system_category()});
        }
        data = data.subspan(static_cast<size_t>(sent));
    }
    return {};
}

auto Connection::recv(std::span<std::byte> buf) -> std::expected<size_t, std::error_code> {
    auto n = ::recv(fd_, buf.data(), buf.size(), 0);
    if (n < 0) {
        return std::unexpected(std::error_code{errno, std::system_category()});
    }
    return static_cast<size_t>(n);
}

} // namespace tinyhttp
