#ifndef CLIENT_H
#define CLIENT_H

#include "message.h"
#include "tsqueue.h"
#include <asio.hpp>
#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <string_view>
#include <tuple>

using asio::ip::tcp;
using namespace std::literals;

typedef uint16_t peer_id;

/*
 * This represents a client in the peer-to-peer network.
 * It needs a port (so that it can listen to other peers)
 * and names (the hostnames and ports of other peers)
 *
 * The constructor will start the cycle function (see below).
 */
class Client {
  public:
    Client(uint16_t port,
           std::vector<std::tuple<std::string, std::string>> names);
    ~Client();

    void push_message(peer_id id, const Message &msg);

    void remove_socket(peer_id id);

  private:
    void accept_socket();
    /* this is the heart of the client
     * this will be called per N seconds (see constructor)
     * so messages can be sent (unimplmented)
     */
    void cycle();
    /*
     * connect to all peers listed in names
     */
    void connect_to_peers();
    /*
     * connect to a single peer, called by connect_to_peers
     * the socket will be added to the peers list if successful
     */
    void connect_to_peer(std::string &host, std::string &service);
    /*
     * what to do when exit signals are generated, like SIGNIT etc.
     */
    void trap_signal();

    asio::io_context ctx;
    // this accepts incoming connections
    tcp::acceptor acceptor;
    // these hold the list of incoming and outgoing connections
    // std::vector<tcp::socket> clients, peers;
    std::map<peer_id, std::shared_ptr<tcp::socket>> peers;
    // resolves the hostname port to a valid endpoint
    tcp::resolver resolver;
    // the timeout function that calls cycle
    asio::high_resolution_timer timer;
    // remove this later, this is just a send message example
    std::string const message = "testing\n";
    // store the host port combinations in the constructor
    std::vector<std::tuple<std::string, std::string>> names;
    // priming the context
    std::thread worker;
    // storing outgoing messages
    ThreadSafeQueue<MessageWithOwner> out_messages;
    peer_id current_id = 1;
};

#endif
