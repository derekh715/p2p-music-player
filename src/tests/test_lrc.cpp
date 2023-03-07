#include "../lrc.h"
#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <utility>
/*
 * NOTE:
 * when ctest is executing, it is in the build directory, so it need to go back
 * one step the paths below are correct
 */

using namespace testing;

TEST(lrc_test, get_lyrics) {
    // this will core dump but the file is valid
    Lrc f("../src/tests/data/jamaica_farewell_first_verse.lrc");
    std::vector<std::string> lines = f.getAllLyrics();
    std::vector<std::string> expected = {
        std::string("Jamaica Farewell"),
        std::string("The Brother Four"),
        std::string(""),
        std::string("Down the way where the nights are gay"),
        std::string("And the sun shines daily on the mountain top"),
        std::string("I took a trip on a sailing ship"),
        std::string("And when I reached Jamaica I made a stop")};
    EXPECT_THAT(lines, ContainerEq(expected));
}

TEST(lrc_test, get_random_lines) {
    // this will not coredump but it always gets empty string?
    // check jamaica_farewell.lrc for the timestamps
    Lrc f("../src/tests/data/jamaica_farewell.lrc");
    auto expected = {
        std::make_pair(std::string("Jamaica Farewell"), 100), // 00:01.00
        std::make_pair(std::string("But I'm sad to say I'm on my way"),
                       35900), // 00:35.90
        std::make_pair(std::string("Down at the market you can hear"),
                       114500), // 01:54.50
        std::make_pair(
            std::string("I had to leave a little girl in Kingston Town"),
            162660),                                 // 02:42.66
        std::make_pair(std::string("End"), 174690)}; // 02:54.69
    for (auto &e : expected) {
        auto line = f.getLyric(e.second);
        EXPECT_THAT(line.s2, Eq(e.first));
    }
}

TEST(lrc_test, enhanced_ignores_inner_timestamps_when_get_lyrics) {
    // when ctest is executing, it is in the build directory
    Lrc f("../src/tests/data/test.lrc");
    // this should be after 06:47 + 500ms
    // so it should by "abc and def..."
    Lyric line = f.getLyric(6971);
    // but it omits "abc"?
    EXPECT_THAT(line.s2, Eq(std::string("abc and def you haha")));
}
