#include "../store.h"
#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(db_test, get_empty_tracks) {
    Store s(true, ":memory:");
    auto entry = s.read_all();
    // no tracks should be found
    EXPECT_THAT(entry, ContainerEq(std::vector<Track>()));
}

TEST(db_test, insert_single_track_works) {
    Store s(true, ":memory:");
    // some random info
    Track t = {.album = "Try to Remember",
               .artist = "The Brothers Four",
               .title = "Jamaica Farewell",
               .duration = (120 + 55) * 1000};
    bool success = s.create(t);
    EXPECT_THAT(success, Eq(true)); // it is really inserted into the database

    // verify that it really is in the database
    // the auto generated id should be one
    auto entry = s.read(1);
    EXPECT_THAT(entry.id, Not(Eq(-1))); // it should exist
    t.id = 1;                           // so the id matches
    EXPECT_THAT(entry, Eq(t));
}

TEST(db_test, get_multiple_tracks) {
    const int size = 20;
    Store s(true, ":memory:");
    for (int i = 0; i < size; i++) {
        Track t;
        t.duration = i;
        s.create(t);
    }

    std::vector<Track> vec = s.read_all();
    EXPECT_THAT(vec.size(), size);
    for (int i = 0; i < size; i++) {
        EXPECT_THAT(vec[i].duration, i);
    }
}

TEST(db_test, removing_invalid_tracks_gives_false) {
    Store s(true, ":memory:");
    // the database is empty, this should not work
    bool success = s.remove(1);
    EXPECT_THAT(success, Eq(false));
}

TEST(db_test, remove_one_from_database_leaves_n_minus_1) {
    Store s(true, ":memory:");
    const int size = 20;
    for (int i = 0; i < size; i++) {
        Track t;
        t.duration = i;
        s.create(t);
    }
    // now the database is not empty, this should work
    // 11 is just any random id
    bool success = s.remove(11);
    EXPECT_THAT(success, Eq(true));

    auto remaining = s.read_all();
    EXPECT_THAT(remaining.size(), Eq(size - 1));
}

TEST(db_test, update_non_existent_track) {
    Store s(true, ":memory:");
    // the database is empty, this should not work
    Track t;
    bool success = s.update(1, t);
    EXPECT_THAT(success, Eq(false));
}

TEST(db_test, update_track_works) {
    Store s(true, ":memory:");
    // the database is empty, this should not work
    Track t = {
        .id = 1,
        .album = "Try to Remember",
        .artist = "The Brothers Four",
        .title = "Jamaica Farewell",
        .duration = (120 + 55) * 1000,
    };
    s.create(t);
    t.album = "Try to NOT Remember";
    s.update(1, t);

    // try to read it again
    Track tt = s.read(1);
    EXPECT_THAT(tt, Eq(t));
}

TEST(db_test, adding_invalid_lrc_file_path) {
    Store s(true, ":memory:");
    // the database is empty, this should not work
    Track t = {
        .lrcfile = "notexist.lrc",
    };
    bool success = s.create(t, true);
    EXPECT_THAT(success, Eq(false));
}

TEST(db_test, adding_valid_lrc_file_path) {
    Store s(true, ":memory:");
    // the database is empty, this should not work
    Track t = {
        .lrcfile = "../src/tests/data/jamaica_farewell.lrc",
    };
    bool success = s.create(t);
    EXPECT_THAT(success, Eq(true));
}

TEST(db_test, matches_nothing) {
    Store s(true, ":memory:");
    auto tracks = s.search("Hello");
    EXPECT_THAT(tracks.empty(), Eq(true));
}

TEST(db_test, matches_one_or_more) {
    Store s(true, ":memory:");
    // some random info
    Track t = {
        .id = 1,
        .album = "Try to Forget",
        .artist = "The Brothers Five",
        .title = "Try to Forget",
        .duration = (120 + 55) * 1000,
    };
    s.create(t);
    auto entries = s.search("five");
    EXPECT_THAT(entries[0], Eq(t)); // it should exist
    entries = s.search("to");
    EXPECT_THAT(entries[0], Eq(t)); // it should exist
    entries = s.search("ve");
    EXPECT_THAT(entries[0], Eq(t)); // it should exist
}

TEST(db_test, bytes_to_hex_string) {
    uint8_t bytes[16] = {0x21, 0x22, 0x23, 0x24, 0x25, 0x26};
    std::string expected = "21222324252600000000000000000000";
    std::string actual = to_hex_string(bytes);
    EXPECT_THAT(actual, Eq(expected));
}

TEST(db_test, addin_real_audio_file) {
    Store s(true, ":memory:");
    Track t = {.title = "File Example",
               .path = "../src/tests/data/file_example.wav"};
    s.create(t);
    auto entries = s.search("File");
    std::cout << entries[0];
    // the create function to calculate these two things
    EXPECT_THAT(entries[0].checksum.empty(), Eq(false));
    EXPECT_THAT(entries[0].path.empty(), Eq(false));
}

TEST(db_test, upsert_as_insert) {
    Store s(true, ":memory:");
    Track t = {
        .title = "File Example",
    };
    s.upsert(t);
    auto entries = s.read_all();
    // so that they are equal
    t.id = 1;
    EXPECT_THAT(entries[0], Eq(t));
}

TEST(db_test, upsert_as_update) {
    Store s(true, ":memory:");
    Track t = {
        .title = "File Example",
    };
    s.upsert(t);
    auto entries = s.read_all();
    entries[0].title = "Changed!";
    // should not create a new entry
    s.upsert(entries[0]);
    auto new_entries = s.read_all();
    EXPECT_THAT(new_entries.size(), 1);
    EXPECT_THAT(new_entries[0], entries[0]);
}
