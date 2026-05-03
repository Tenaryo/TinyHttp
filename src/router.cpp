#include "router.hpp"

#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace tinyhttp {

Router::Router(std::filesystem::path directory) : directory_(std::move(directory)) {}

auto Router::dispatch(const Request& req) const -> Response {
    Response resp;

    if (req.path == "/") {
        resp.set_status(200, "OK");
    } else if (auto echo = match_echo_path(req.path)) {
        resp.set_status(200, "OK");
        resp.add_header("Content-Type", "text/plain");
        auto body = std::string(*echo);
        resp.add_header("Content-Length", std::to_string(body.size()));
        resp.set_body({reinterpret_cast<const std::byte*>(body.data()), body.size()});
    } else if (req.path == "/user-agent") {
        resp.set_status(200, "OK");
        resp.add_header("Content-Type", "text/plain");
        auto ua = std::string(req.get_header("User-Agent").value_or(""));
        resp.add_header("Content-Length", std::to_string(ua.size()));
        resp.set_body({reinterpret_cast<const std::byte*>(ua.data()), ua.size()});
    } else if (req.path.starts_with("/files/")) {
        auto filename = req.path.substr(std::string_view("/files/").size());
        if (filename.empty() || filename.find('/') != std::string_view::npos) {
            resp.set_status(404, "Not Found");
        } else {
            resp = serve_file(filename);
        }
    } else {
        resp.set_status(404, "Not Found");
    }

    return resp;
}

auto Router::serve_file(std::string_view filename) const -> Response {
    Response resp;
    auto filepath = directory_ / filename;

    int fd = ::open(filepath.c_str(), O_RDONLY);
    if (fd < 0) {
        resp.set_status(404, "Not Found");
        return resp;
    }

    struct stat st {};
    if (::fstat(fd, &st) < 0) {
        ::close(fd);
        resp.set_status(404, "Not Found");
        return resp;
    }

    if (!S_ISREG(st.st_mode)) {
        ::close(fd);
        resp.set_status(404, "Not Found");
        return resp;
    }

    auto size = static_cast<std::size_t>(st.st_size);

    void* mapped = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd);

    if (mapped == MAP_FAILED) {
        resp.set_status(404, "Not Found");
        return resp;
    }

    resp.set_status(200, "OK");
    resp.add_header("Content-Type", "application/octet-stream");
    resp.add_header("Content-Length", std::to_string(size));
    resp.set_body({static_cast<const std::byte*>(mapped), size});

    ::munmap(mapped, size);
    return resp;
}

} // namespace tinyhttp
