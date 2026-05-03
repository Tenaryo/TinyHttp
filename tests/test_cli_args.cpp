#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "config.hpp"

TEST(ParseArgs, DirectoryFlag) {
    std::string_view args[] = {"--directory", "/tmp/data"};
    auto result = tinyhttp::parse_args(args);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->directory, "/tmp/data");
}

TEST(ParseArgs, UnknownFlag) {
    std::string_view args[] = {"--unknown", "value"};
    auto result = tinyhttp::parse_args(args);
    EXPECT_FALSE(result.has_value());
}

TEST(ParseArgs, MissingDirectoryValue) {
    std::string_view args[] = {"--directory"};
    auto result = tinyhttp::parse_args(args);
    EXPECT_FALSE(result.has_value());
}
