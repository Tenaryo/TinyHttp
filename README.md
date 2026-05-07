# TinyHttp

[![CI](https://github.com/Tenaryo/TinyHttp/actions/workflows/ci.yml/badge.svg)](https://github.com/Tenaryo/TinyHttp/actions/workflows/ci.yml)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

A minimal, educational HTTP/1.1 server written in modern C++23.

## Features

- HTTP/1.1 request parsing with case-insensitive header lookup
- Route dispatcher: `/` (200), `/echo/<str>`, `/user-agent`, `/files/<name>`
- `GET /files/<name>` serves files via **memory-mapped I/O (mmap)**
- `POST /files/<name>` writes request body to disk
- **gzip compression** with `Accept-Encoding` / `Content-Encoding` negotiation
- **Persistent connections** (HTTP keep-alive) with `Connection: close` handling
- **Fixed-size thread pool** for bounded concurrency
- Path traversal protection on `/files/` routes

## Quick Start

### Prerequisites

- **GCC 13+** (or Clang 17+)
- **CMake 3.21+**
- **Ninja**
- **zlib**

```bash
# Ubuntu / Debian
sudo apt install g++-13 ninja-build cmake zlib1g-dev
```

### Build

```bash
./build.sh          # Debug build
./build.sh Release  # Release build with -O3
```

The binary is at `build/http-server`.

### Run

```bash
./build/http-server
```

The server listens on port 4221.

Serve files from a directory:

```bash
./build/http-server --directory /path/to/files
```

Enable sanitizers for development:

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build -j$(nproc)
```

### Test

```bash
./run_tests.sh
```

## Usage

```bash
# Basic
curl -v http://localhost:4221/

# Echo
curl -v http://localhost:4221/echo/hello-world

# User-Agent
curl -v -H "User-Agent: TinyHttp/1.0" http://localhost:4221/user-agent

# Gzip compression
curl -v -H "Accept-Encoding: gzip" http://localhost:4221/echo/hello | gunzip

# File download
./build/http-server --directory /tmp
echo "Hello, World!" > /tmp/foo
curl -v http://localhost:4221/files/foo

# File upload
curl -v -X POST --data-binary "hello" http://localhost:4221/files/bar

# Persistent connection
printf "GET / HTTP/1.1\r\nHost: localhost\r\n\r\nGET /echo/abc HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 4221
```

## Implementation Highlights

**Zero-copy memory-mapped file serving** &mdash; Static files are served via `mmap` with `MAP_PRIVATE`, bypassing userspace buffers and leveraging the kernel page cache.

**Fixed thread pool with C++23 `std::move_only_function`** &mdash; A lightweight work-stealing thread pool caps concurrency at `std::thread::hardware_concurrency()`, avoiding unbounded thread creation under load.

**No-exception error handling with `std::expected`** &mdash; All fallible operations return `std::expected<T, E>` (C++23). No hidden control flow, no surprise unwinding &mdash; every error path is explicit.

**Zero-copy HTTP parsing** &mdash; `parse_request` operates entirely on `std::string_view` over the raw receive buffer. Method, path, version, headers, and body are all non-owning views. No allocations during parsing.

**gzip compression with raw zlib C API** &mdash; Uses `deflateInit2` with gzip wrapper bits (`15 + 16`) for standards-compliant gzip output. Compression results are moved directly into the response body &mdash; zero copy from compress to wire.

**RAII socket management** &mdash; `Connection` and `Server` classes manage file descriptors with move semantics and deterministic close in destructors. `MSG_NOSIGNAL` prevents SIGPIPE on client disconnect.

## License

[MIT](LICENSE)
