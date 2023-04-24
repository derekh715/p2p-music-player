#include "../file-sharing.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(test_filesharing, pausing_and_playing) {
    FileSharing f;
    int id = f.new_peer(1);
    EXPECT_EQ(f.paused(), false);

    f.pause_writing();
    EXPECT_EQ(f.paused(), true);
    EXPECT_EQ(f.is_peer_idle(id), true);

    f.resume_writing();
    EXPECT_EQ(f.paused(), false);
    f.if_idle([](int assigned_id) {});
    EXPECT_EQ(f.is_peer_idle(id), false);
}

TEST(test_filesharing, make_it_die) {
    FileSharing f;
    int id = f.new_peer(1);
    f.increment_peer_failure(id);
    f.increment_peer_failure(id);
    f.increment_peer_failure(id);
    f.increment_peer_failure(id);
    EXPECT_EQ(f.is_peer_dead(id), true);
}
