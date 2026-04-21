#include <cassert>
#include <iostream>
#include <string_view>

#include "request.hpp"

auto test_parse_root_path() -> void {
    auto result = tinyhttp::parse_request("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    assert(result.has_value() && "parse should succeed");
    assert(result->method == "GET");
    assert(result->path == "/");
    assert(result->version == "HTTP/1.1");
    std::cout << "test_parse_root_path: PASSED\n";
}

auto test_parse_arbitrary_path() -> void {
    auto result = tinyhttp::parse_request("GET /abcdefg HTTP/1.1\r\nHost: localhost\r\n\r\n");
    assert(result.has_value() && "parse should succeed");
    assert(result->method == "GET");
    assert(result->path == "/abcdefg");
    assert(result->version == "HTTP/1.1");
    std::cout << "test_parse_arbitrary_path: PASSED\n";
}

auto test_parse_missing_crlf() -> void {
    auto result = tinyhttp::parse_request("GET / HTTP/1.1");
    assert(!result.has_value() && "parse should fail without CRLF");
    std::cout << "test_parse_missing_crlf: PASSED\n";
}

auto test_parse_malformed_request() -> void {
    auto result = tinyhttp::parse_request("INVALID\r\n");
    assert(!result.has_value() && "parse should fail for malformed request");
    std::cout << "test_parse_malformed_request: PASSED\n";
}

auto main() -> int {
    test_parse_root_path();
    test_parse_arbitrary_path();
    test_parse_missing_crlf();
    test_parse_malformed_request();
    std::cout << "All tests passed!\n";
    return 0;
}
