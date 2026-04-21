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
