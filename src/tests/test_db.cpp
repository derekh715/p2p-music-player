#include "../store.h"
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
               .author = "The Brothers Four",
               .title = "Jamaica Farewell",
               .len = (120 + 55) * 1000};
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
        t.len = i;
        s.create(t);
    }

    std::vector<Track> vec = s.read_all();
    EXPECT_THAT(vec.size(), size);
    for (int i = 0; i < size; i++) {
        EXPECT_THAT(vec[i].len, i);
    }
}
