#ifndef BASE_CLIENT_H
#define BASE_CLIENT_H

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

struct ConnectionInfo {
    std::string address;
    uint16_t port;
};

/*
 * This represents a client in the peer-to-peer network.
 * It needs a port (so that it can listen to other peers)
 *
 * The constructor will start the cycle function (see below).
 */
class BaseClient {
  public:
    BaseClient(uint16_t port, std::chrono::milliseconds cycle_time = 1000ms);
    ~BaseClient();

    void push_message(peer_id id, const Message &msg);

    void remove_socket(peer_id id);
    void remove_socket(std::shared_ptr<tcp::socket> socket);
    void remove_socket_by_ip(const std::string &address, uint16_t port);

    /*
     * connect to a single peer, called by connect_to_peers
     * the socket will be added to the peers list if successful
     * it will return false the resolver fails
     */
    bool connect_to_peer(const std::string &host, const std::string &service);

    /*
     * this sends a message to ALL clients
     */
    void broadcast(const Message &msg);

    /*
     * get all currently connected peer_id, socket pairs
     */
    std::vector<std::pair<peer_id, std::shared_ptr<tcp::socket>>> get_sockets();

  protected:
    void accept_socket();

    void add_to_peers(std::shared_ptr<tcp::socket> socket);
    /* this is the heart of the client
     * this will be called per N seconds (see constructor)
     * so messages can be sent (unimplmented)
     */
    void cycle();
    /*
     * what to do when exit signals are generated, like SIGNIT etc.
     */
    void trap_signal();

    /*
     * Prime the context to read messages from peers
     */
    void start_reading(std::shared_ptr<tcp::socket> socket);

    /*
     * Read the message body if there is one
     */
    void read_body(std::shared_ptr<tcp::socket> socket);

    /*
     * Pushed the read message into the in queue
     */
    void add_to_incoming(std::shared_ptr<tcp::socket> socket);

    /*
     * Start writing the message to a socket
     */
    void start_writing();

    /*
     * Write the message body if there is one
     */
    void write_body(std::shared_ptr<tcp::socket> socket);

    /*
     * this is the HEART of the class
     * it takes in a message from any of the peers
     * MessageWithOwner will give the id and the entire message
     * see the PING and PONG examples of distinguishing different message types
     */
    virtual void handle_message(MessageWithOwner &msg) = 0;

    asio::io_context ctx;
    // this accepts incoming connections
    tcp::acceptor acceptor;
    // these hold the list of incoming and outgoing connections
    // std::vector<tcp::socket> clients, peers;
    std::map<peer_id, std::shared_ptr<tcp::socket>> peers;
    std::map<std::shared_ptr<tcp::socket>, peer_id> reverse;
    std::map<peer_id, ConnectionInfo> peer_ip_map;
    // resolves the hostname port to a valid endpoint
    tcp::resolver resolver;
    // the timeout function that calls cycle
    asio::high_resolution_timer timer;
    // priming the context
    std::thread worker;
    // storing outgoing messages
    ThreadSafeQueue<MessageWithOwner> out_msgs;
    ThreadSafeQueue<MessageWithOwner> in_msgs;
    peer_id current_id = 1;
    Message temp_msg;
    std::chrono::milliseconds cycle_time;
};

#endif
