#include <algorithm>
#include <deque>
#include <mutex>

/*
 * A thread safe wrapper around the standard library deque
 */
template <typename T> class ThreadSafeQueue {
  public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue<T> &t) = delete;
    ~ThreadSafeQueue() { q.clear(); }

    const T &front() {
        std::scoped_lock l(mux);
        return q.front();
    }

    const T &back() {
        std::scoped_lock l(mux);
        return q.back();
    }

    T pop_front() {
        std::scoped_lock l(mux);
        // it needs to be done since pop_front() returns nothing
        auto rref = std::move(q.front());
        q.pop_front();
        return rref;
    }

    T pop_back() {
        std::scoped_lock l(mux);
        auto rref = std::move(q.back());
        q.pop_back();
        return rref;
    }

    void push_back(const T &item) {
        std::scoped_lock l(mux);
        q.push_back(item);
    }

    void push_front(const T &item) {
        std::scoped_lock l(mux);
        q.push_front(item);
    }

    bool empty() {
        std::scoped_lock l(mux);
        return q.empty();
    }

    size_t count() {
        std::scoped_lock l(mux);
        return q.size();
    }

    void clear() {
        std::scoped_lock l(mux);
        return q.clear();
    }

  private:
    std::mutex mux;
    std::deque<T> q;
};
