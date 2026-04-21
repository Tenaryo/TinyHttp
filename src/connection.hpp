#pragma once

#include <cstddef>
#include <expected>
#include <span>
#include <system_error>

namespace tinyhttp {

class Connection {
  public:
    explicit Connection(int fd) noexcept;
    Connection(Connection&& other) noexcept;
    auto operator=(Connection&& other) noexcept -> Connection&;
    Connection(const Connection&) = delete;
    auto operator=(const Connection&) -> Connection& = delete;
    ~Connection();

    auto send(std::span<const std::byte> data) -> std::expected<void, std::error_code>;
    auto recv(std::span<std::byte> buf) -> std::expected<size_t, std::error_code>;
  private:
    int fd_;
};

} // namespace tinyhttp
