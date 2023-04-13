#include "client.h"

Client::Client(uint16_t port,
               std::vector<std::tuple<std::string, std::string>> names)
    : acceptor(ctx, tcp::endpoint(tcp::v4(), port)), resolver(ctx), timer(ctx),
      names(names) {
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    acceptor.listen();
    trap_signal();
    accept_socket();
    connect_to_peers();
    cycle();
    worker = std::thread([this]() { ctx.run(); });
}

Client::~Client() {
    if (worker.joinable()) {
        worker.join();
    }
}

void Client::trap_signal() {
    asio::signal_set sigs(ctx, SIGINT, SIGTERM);

    sigs.async_wait([&](asio::error_code ec, int signal) {
        if (!ec) {
            std::cout << "Signal received: " << signal << std::endl;
            acceptor.cancel();
            timer.cancel();
        } else {
            std::cout << std::endl << ec.message() << std::endl;
        }
    });
}

void Client::accept_socket() {
    acceptor.async_accept([&](asio::error_code const &ec, tcp::socket socket) {
        if (!ec) {
            std::cout << "New session: " << socket.remote_endpoint()
                      << std::endl;
            peers[current_id++] = std::make_shared<tcp::socket>(ctx);
            accept_socket();
        } else {
            std::cout << ec.message() << std::endl;
        }
    });
}

void Client::connect_to_peers() {
    for (auto &name : names) {
        connect_to_peer(std::get<0>(name), std::get<1>(name));
    }
}

void Client::connect_to_peer(std::string &host, std::string &service) {
    peers[current_id++] = std::make_shared<tcp::socket>(ctx);
    auto &p = peers[current_id - 1];
    auto endpoints = resolver.resolve(host, service);
    asio::async_connect(*p, endpoints,
                        [&, spec = (host + ":"s + service)](
                            asio::error_code ec, tcp::endpoint endpoint) {
                            std::cout << "For: " << spec << " " << ec.message();
                            if (!ec) {
                                std::cout << " " << p->remote_endpoint();
                            } else {
                                peers.erase(peers.find(current_id - 1));
                            }
                            std::cout << std::endl;
                        });
}

void Client::push_message(peer_id id, const Message &msg) {
    out_messages.push_back({msg, id});
}

void Client::remove_socket(peer_id id) {
    auto it = peers.find(id);
    if (it != peers.end()) {
        peers.erase(it);
    }
}

void Client::cycle() {
    timer.expires_from_now(2s);
    timer.async_wait([&](asio::error_code ec) {
        if (ec) {
            std::cout << "Something is wrong with the timer: " << ec.message()
                      << std::endl;
            return;
        }
        asio::error_code ec2;
        while (!out_messages.empty()) {
            auto t = out_messages.pop_front();
            auto it = peers.find(t.id);
            if (it == peers.end()) {
                std::cout << "The output message has invalid peer id."
                          << std::endl;
                continue;
            }
            auto &socket = it->second;
            auto remote = socket->remote_endpoint(ec2);
            if (ec2) {
                remove_socket(t.id);
                std::cout << "Removed a peer from the peers collection as "
                             "it is down"
                          << std::endl;
                continue;
            }
            asio::async_write(*socket, asio::buffer(message),
                              [ep = socket->remote_endpoint(ec)](
                                  asio::error_code ec, size_t len) {
                                  std::cout << "Sent " << len << " bytes to"
                                            << ep << " " << ec.message()
                                            << std::endl;
                              });
        }
        cycle();
    });
}
