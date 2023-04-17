#include "../chunked-file.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(test_chunk, reading_non_existent_file) {
    ChunkedFile cf("doesn't exist");
    EXPECT_EQ(cf.failure(), true);
    std::vector<char> s;
    EXPECT_EQ(cf.get(0, s), false);
}

TEST(test_chunk, reading_each_segment) {
    // each segment is four bytes
    ChunkedFile cf("../src/tests/data/ascii_chunk.txt", 4);
    EXPECT_EQ(cf.failure(), false);
    // the ascii file looks like AAAABBBBCCCC ... GGGG so there are 7 parts
    // the last byte is LF ending
    EXPECT_EQ(cf.total_segments, 8);
    std::vector<char> s(4);
    for (int i = 0; i < 7; i++) {
        cf.get(i, s);
        EXPECT_THAT(s, ContainerEq(std::vector<char>(4, 'A' + i)));
    }
    cf.get(7, s);
    EXPECT_THAT(s, ContainerEq(std::vector<char>(1, '\n')));
}

TEST(test_chunk, reading_invalid_segments) {
    // each segment is four bytes
    ChunkedFile cf("../src/tests/data/ascii_chunk.txt", 4);
    std::vector<char> s(4);
    // just one after the last
    bool fine = cf.get(8, s);
    EXPECT_THAT(fine, Eq(false));
    fine = cf.get(-1, s);
    EXPECT_THAT(fine, Eq(false));
    fine = cf.get(1000212, s);
    EXPECT_THAT(fine, Eq(false));
    fine = cf.get(-11133, s);
    EXPECT_THAT(fine, Eq(false));
}
