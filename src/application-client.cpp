#include "application-client.h"

ApplicationClient::ApplicationClient(
    uint16_t port, std::function<void(MessageWithOwner &)> handler,
    std::function<void(peer_id)> connect,
    std::function<void(peer_id)> disconnect,
    std::function<void()> additional_cycle_hook)
    : BaseClient(port), handler{handler}, connect_handler{connect},
      disconnect_handler{disconnect}, additional_cycle_hook{
                                          additional_cycle_hook} {
    cycle();
}

ApplicationClient::~ApplicationClient() {}

void ApplicationClient::handle_message(MessageWithOwner &msg) { handler(msg); }

void ApplicationClient::cycle() {
    timer.expires_from_now(cycle_time);
    timer.async_wait([&](asio::error_code ec) {
        additional_cycle_hook();
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

void ApplicationClient::on_connect(peer_id id) { connect_handler(id); }
void ApplicationClient::on_disconnect(peer_id id) { disconnect_handler(id); }
