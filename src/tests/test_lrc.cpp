#include "../lrc.h"
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
        EXPECT_THAT(line.s1, Eq(e.first));
    }
}

TEST(lrc_test, enhanced_ignores_inner_timestamps_when_get_lyrics) {
    Lrc f("../src/tests/data/test.lrc");
    // this should be after 06:47 + 500ms
    Lyric line = f.getLyric(6971);
    EXPECT_THAT(line.s1, Eq(std::string("Abc ")));
    EXPECT_THAT(line.s2, Eq(std::string("and def you haha")));
}

TEST(lrc_test, invalid_file) {
    Lrc f("this_file_doesnt_exist!");
    EXPECT_THAT(f.failed(), Eq(true));
}

TEST(lrc_test, reading_lyrics_other_than_ascii) {
    // just test a few lines is enough
    Lrc f("../src/tests/data/apple_of_my_eye.lrc");
    std::vector<std::string> lines = f.getAllLyrics();
    lines = std::vector<std::string>(lines.begin(), lines.begin() + 6);
    std::vector<std::string> expected = {
        std::string("又回到最初的起點"),
        std::string("記憶中妳青澀的臉"),
        std::string("我們終於來到了這一天"),
        std::string("桌墊下的老照片"),
        std::string("無數回憶連結"),
        std::string("今天男孩要赴女孩最後的約"),
    };
    EXPECT_THAT(lines, ContainerEq(expected));
    Lrc g("../src/tests/data/ikenai_taiyou.lrc");
    lines = g.getAllLyrics();
    lines = std::vector<std::string>(lines.end() - 10, lines.end() - 5);
    expected = {
        std::string("ABC 続かない　そんなんじゃ　ダメじゃない"),
        std::string("だって　ココロの奧は違うんじゃない？"),
        std::string("オレの青春　そんなもんじゃない"),
        std::string("熱く奧で果てたいよ"),
        std::string("きっと　キミじゃなきゃ　やだよ"),
    };
    EXPECT_THAT(lines, ContainerEq(expected));
}
