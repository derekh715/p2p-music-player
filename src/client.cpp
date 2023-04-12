#include "client.h"

Client::Client(uint16_t port,
               std::list<std::tuple<std::string, std::string>> names)
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
            clients.push_back(std::move(socket));
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
    auto &p = peers.emplace_back(ctx);
    auto endpoints = resolver.resolve(host, service);
    asio::async_connect(
        p, endpoints,
        [&, spec = (host + ":"s + service)](asio::error_code ec,
                                            tcp::endpoint endpoint) {
            std::cout << "For: " << spec << " " << ec.message();
            if (!ec) {
                std::cout << " " << p.remote_endpoint();
            } else {
                peers.remove_if([&p](auto const &lhs) { return &lhs == &p; });
            }
            std::cout << std::endl;
        });
}

void Client::cycle() {
    timer.expires_from_now(2s);
    timer.async_wait([&](asio::error_code ec) {
        if (!ec) {
            asio::error_code ec2;
            for (auto &peer : peers) {
                auto remote = peer.remote_endpoint(ec2);
                if (ec2) {
                    std::cout << "Peer down!" << std::endl;
                    continue;
                }
                std::cout << "Sending to " << remote << std::endl;
                asio::async_write(peer, asio::buffer(message),
                                  [ep = peer.remote_endpoint(ec)](
                                      asio::error_code ec, size_t len) {
                                      std::cout << "Sent " << len << " bytes to"
                                                << ep << " " << ec.message()
                                                << std::endl;
                                  });
            }
        }
        cycle();
    });
}
