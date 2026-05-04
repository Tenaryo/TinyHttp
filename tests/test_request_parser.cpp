#include <cstddef>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>
#include <zlib.h>

#include "gzip.hpp"
#include "request.hpp"

TEST(ParseRequest, RootPath) {
    auto result = tinyhttp::parse_request("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, "GET");
    EXPECT_EQ(result->path, "/");
    EXPECT_EQ(result->version, "HTTP/1.1");
}

TEST(ParseRequest, ArbitraryPath) {
    auto result = tinyhttp::parse_request("GET /abcdefg HTTP/1.1\r\nHost: localhost\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->method, "GET");
    EXPECT_EQ(result->path, "/abcdefg");
    EXPECT_EQ(result->version, "HTTP/1.1");
}

TEST(ParseRequest, MissingCRLF) {
    auto result = tinyhttp::parse_request("GET / HTTP/1.1");
    EXPECT_FALSE(result.has_value());
}

TEST(ParseRequest, MalformedRequest) {
    auto result = tinyhttp::parse_request("INVALID\r\n");
    EXPECT_FALSE(result.has_value());
}

TEST(MatchEchoPath, ExtractsEchoString) {
    auto result = tinyhttp::match_echo_path("/echo/abc");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "abc");
}

TEST(MatchEchoPath, ExtractsMultiSegment) {
    auto result = tinyhttp::match_echo_path("/echo/hello-world");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello-world");
}

TEST(MatchEchoPath, EmptySuffixReturnsEmpty) {
    auto result = tinyhttp::match_echo_path("/echo/");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "");
}

TEST(MatchEchoPath, NoTrailingSlashReturnsNullopt) {
    auto result = tinyhttp::match_echo_path("/echo");
    EXPECT_FALSE(result.has_value());
}

TEST(ParseRequest, ExtractsHeaders) {
    auto result = tinyhttp::parse_request(
        "GET /user-agent HTTP/1.1\r\nHost: localhost\r\nUser-Agent: foobar/1.2.3\r\nAccept: */*\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get_header("User-Agent"), "foobar/1.2.3");
    EXPECT_EQ(result->get_header("Host"), "localhost");
    EXPECT_EQ(result->get_header("Accept"), "*/*");
    EXPECT_EQ(result->get_header("X-Not-Exist"), std::nullopt);
}

TEST(ParseRequest, CaseInsensitiveHeaderLookup) {
    auto result = tinyhttp::parse_request(
        "GET /user-agent HTTP/1.1\r\nHost: localhost\r\nUser-Agent: foobar/1.2.3\r\n\r\n");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get_header("user-agent"), "foobar/1.2.3");
    EXPECT_EQ(result->get_header("USER-AGENT"), "foobar/1.2.3");
    EXPECT_EQ(result->get_header("User-Agent"), "foobar/1.2.3");
    EXPECT_EQ(result->get_header("uSeR-aGeNt"), "foobar/1.2.3");
}

TEST(MatchEchoPath, DifferentPrefixReturnsNullopt) {
    auto result = tinyhttp::match_echo_path("/other/abc");
    EXPECT_FALSE(result.has_value());
}

TEST(ParseRequest, BodyFromContentLength) {
    auto result = tinyhttp::parse_request(
        "POST /files/test HTTP/1.1\r\nContent-Length: 5\r\n\r\n12345");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->body, "12345");
}

TEST(GzipCompression, RoundTrip) {
    auto decompress = [](const std::vector<std::byte>& compressed) -> std::string {
        z_stream strm{};
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<std::byte*>(compressed.data()));
        strm.avail_in = static_cast<uInt>(compressed.size());

        int rc = inflateInit2(&strm, 16 + MAX_WBITS);
        EXPECT_EQ(rc, Z_OK) << "inflateInit2 failed";

        std::string result;
        char buf[256];
        do {
            strm.next_out = reinterpret_cast<Bytef*>(buf);
            strm.avail_out = sizeof(buf);
            rc = inflate(&strm, Z_NO_FLUSH);
            if (rc == Z_STREAM_ERROR || rc == Z_DATA_ERROR || rc == Z_MEM_ERROR) {
                inflateEnd(&strm);
                ADD_FAILURE() << "inflate failed: " << rc;
                return "";
            }
            result.append(buf, sizeof(buf) - strm.avail_out);
        } while (rc != Z_STREAM_END);

        inflateEnd(&strm);
        return result;
    };

    auto compressed = tinyhttp::compress_gzip("abc");
    EXPECT_EQ(decompress(compressed), "abc");

    auto compressed2 = tinyhttp::compress_gzip("hello world");
    EXPECT_EQ(decompress(compressed2), "hello world");

    auto compressed3 = tinyhttp::compress_gzip("");
    EXPECT_EQ(decompress(compressed3), "");
}
