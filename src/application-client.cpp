#include "application-client.h"

ApplicationClient::ApplicationClient(
    uint16_t port, std::function<void(MessageWithOwner &)> handler)
    : BaseClient(port), handler{handler} {
    cycle();
}

ApplicationClient::~ApplicationClient() {}

void ApplicationClient::handle_message(MessageWithOwner &msg) { handler(msg); }

void ApplicationClient::cycle() {
    timer.expires_from_now(cycle_time);
    timer.async_wait([&](asio::error_code ec) {
        // clear the in messages array first, if there are messages clear them
        while (!in_msgs.empty()) {
            auto msg = in_msgs.pop_front();
            handle_message(msg);
        }
        // now we clear the out messages array
        start_writing();
        cycle();
    });
}
