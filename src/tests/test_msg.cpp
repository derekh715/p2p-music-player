#include "../message.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

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

TEST(test_msg, pushing_and_pulling_vectors_simple_types) {
    std::vector<int> expect;
    expect.push_back(331501851);
    expect.push_back(-13513508);
    Message m;
    m << expect;
    std::vector<int> actual;
    m >> actual;

    EXPECT_THAT(actual, testing::ContainerEq(expect));
}

TEST(test_msg, pushing_and_pulling_vectors_composite_types) {
    std::vector<std::string> expect;
    expect.push_back("I am fine!");
    expect.push_back("Thank you!");
    Message m;
    m << expect;
    std::vector<std::string> actual;
    m >> actual;

    EXPECT_THAT(actual, testing::ContainerEq(expect));
}

TEST(test_msg, pushing_and_pulling_return_track_info) {
    ReturnTrackInfo expect;
    expect.title = "Random title";
    std::vector<Track> expected_tracks;
    expected_tracks.push_back(Track{.title = "Title 1"});
    expected_tracks.push_back(Track{.title = "Title 2"});
    expect.tracks = expected_tracks;
    Message m;
    m << expect;
    ReturnTrackInfo actual;
    m >> actual;

    EXPECT_EQ(actual.title, expect.title);
    EXPECT_THAT(actual.tracks, testing::ContainerEq(expect.tracks));
}

TEST(test_msg, pushing_and_pulling_lrc) {
    Lrc expect("../src/tests/data/jamaica_farewell_first_verse.lrc");
    Message m;
    m << expect;

    Lrc actual;
    m >> actual;

    // test a few fields
    EXPECT_EQ(actual.lys, expect.lys);
    EXPECT_EQ(actual.album, expect.album);
    EXPECT_EQ(actual.editor, expect.editor);
}
