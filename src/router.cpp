#include "router.hpp"

#include "encoding.hpp"
#include "gzip.hpp"

#include <charconv>
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

        if (auto enc = negotiate_encoding(req); enc == "gzip") {
            if (auto compressed = compress_gzip(body)) {
                resp.add_header("Content-Encoding", "gzip");
                resp.add_header("Content-Length", std::to_string(compressed->size()));
                resp.set_body(*compressed);
                return resp;
            }
        }

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
        } else if (req.method == "GET") {
            resp = serve_file(filename);
        } else if (req.method == "POST") {
            auto cl = req.get_header("Content-Length");
            if (!cl) {
                resp.set_status(400, "Bad Request");
            } else {
                size_t length = 0;
                auto [ptr, ec] = std::from_chars(cl->data(), cl->data() + cl->size(), length);
                if (ec != std::errc{}) {
                    resp.set_status(400, "Bad Request");
                } else {
                    resp = store_file(filename, req.body);
                }
            }
        } else {
            resp.set_status(404, "Not Found");
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

auto Router::store_file(std::string_view filename, std::string_view content) const -> Response {
    Response resp;
    auto filepath = directory_ / filename;

    int fd = ::open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        resp.set_status(500, "Internal Server Error");
        return resp;
    }

    auto remaining = content.size();
    auto ptr = content.data();
    while (remaining > 0) {
        auto written = ::write(fd, ptr, remaining);
        if (written < 0) {
            ::close(fd);
            resp.set_status(500, "Internal Server Error");
            return resp;
        }
        ptr += static_cast<size_t>(written);
        remaining -= static_cast<size_t>(written);
    }

    ::close(fd);
    resp.set_status(201, "Created");
    return resp;
}

} // namespace tinyhttp
