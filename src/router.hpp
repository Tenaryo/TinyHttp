#pragma once

#include "request.hpp"
#include "response.hpp"

#include <filesystem>

namespace tinyhttp {

class Router {
  public:
    explicit Router(std::filesystem::path directory);

    auto dispatch(const Request& req) const -> Response;
  private:
    std::filesystem::path directory_;

    auto serve_file(std::string_view filename) const -> Response;
    auto store_file(std::string_view filename, std::string_view content) const -> Response;
};

} // namespace tinyhttp
