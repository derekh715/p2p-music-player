#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "../message.h"

TEST(test_msg, pushing_and_pulling_single_value) {
    double expect = 3.1415926;
    Message m;
    m << expect;
    double actual;
    m >> actual;
    EXPECT_EQ(expect, actual);
}

TEST(test_msg, pushing_and_pulling_strings) {
    std::string expect = "The quick brown fox jumps over the lazy dog.";
    Message m;
    m << expect;
    std::string actual;
    m >> actual;
    EXPECT_EQ(expect, actual);
}

TEST(test_msg, pushing_and_pulling_strings_multiple) {
    std::string expect1 = "The quick brown fox jumps over the lazy dog.";
    std::string expect2 = "dog lazy the over jumps fox brown quick The.";
    std::string actual1, actual2;
    Message m;
    m << expect1 << expect2;
    std::string actual;
    m >> actual2 >> actual1;
    EXPECT_EQ(expect1, actual1);
    EXPECT_EQ(expect2, actual2);
}

TEST(test_msg, pushing_and_pulling_struct_with_string) {
    Track expect = {
        .id = 11,
        .album = "Test Album",
        .artist = "Artist 1",
        .author = "It is fine",
        .title = "Title",
        .lrcfile = "random path",
        .len = 13313,
    };
    Message m;
    m << expect;
    Track actual;
    m >> actual;

    EXPECT_EQ(actual, expect);
}
