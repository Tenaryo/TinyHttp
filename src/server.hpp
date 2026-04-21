#pragma once

#include "connection.hpp"

#include <cstdint>
#include <expected>
#include <string_view>
#include <system_error>

namespace tinyhttp {

class Server {
  public:
    explicit Server(std::string_view host, uint16_t port);
    Server(const Server&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    ~Server();

    auto listen() -> std::expected<void, std::error_code>;
    auto accept() -> std::expected<Connection, std::error_code>;
  private:
    int fd_{-1};
    uint16_t port_;
};

} // namespace tinyhttp
