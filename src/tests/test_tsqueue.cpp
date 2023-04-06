#include "../tsqueue.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace testing;

TEST(test_queue, no_thread) {
    ThreadSafeQueue<int> tint;
    tint.push_back(1);
    tint.push_back(2);
    tint.push_back(3);

    // there should be three elements
    EXPECT_EQ(tint.count(), 3);

    int i = tint.pop_front();
    EXPECT_EQ(i, 1);

    tint.pop_front();
    tint.pop_front();

    EXPECT_EQ(tint.empty(), true);
}

TEST(test_queue, with_one_thread) {
    ThreadSafeQueue<int> tint;
    std::thread t([&]() {
        tint.push_front(1);
        tint.push_front(2);
        tint.push_front(4);

        // there should be three elements
        EXPECT_EQ(tint.count(), 3);
        EXPECT_EQ(tint.front(), 4);
        EXPECT_EQ(tint.back(), 1);

        int i = tint.pop_front();
        EXPECT_EQ(i, 4);

        int ii = tint.pop_back();
        EXPECT_EQ(ii, 1);

        tint.clear();

        EXPECT_EQ(tint.empty(), true);
        });

    t.join();
}

TEST(test_queue, two_threads_front) {
    ThreadSafeQueue<int> tint;
    tint.push_front(10);
    tint.push_front(20);
    tint.push_front(30);

    std::thread t1([&]() {
        tint.pop_front();
    });

    std::thread t2([&]() {
        tint.pop_front();
    });

    t1.join();
    t2.join();

    EXPECT_THAT(tint.front(), 10);
}

TEST(test_queue, two_threads_back) {
    ThreadSafeQueue<int> tint;
    tint.push_back(10);
    tint.push_back(20);
    tint.push_back(30);

    std::thread t1([&]() {
        tint.pop_back();
    });

    std::thread t2([&]() {
        tint.pop_back();
    });

    t1.join();
    t2.join();

    EXPECT_THAT(tint.back(), 10);
}

TEST(test_queue, two_threads_pushing) {
    ThreadSafeQueue<int> tint;

    std::thread t1([&]() {
        tint.push_back(10);
        tint.push_back(20);
    });

    std::thread t2([&]() {
        tint.push_back(20);
        tint.push_back(30);
    });

    t1.join();
    t2.join();

    // we don't know what order they will be, but there must be four elements
    EXPECT_THAT(tint.count(), 4);
}
