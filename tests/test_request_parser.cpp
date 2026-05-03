#include <optional>
#include <string_view>

#include <gtest/gtest.h>

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
