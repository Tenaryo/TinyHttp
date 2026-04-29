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

TEST(MatchEchoPath, DifferentPrefixReturnsNullopt) {
    auto result = tinyhttp::match_echo_path("/other/abc");
    EXPECT_FALSE(result.has_value());
}
