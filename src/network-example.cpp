#include "client.h"
#include <sstream>
#include <thread>

void get_ip_and_connect(Client &c) {
    std::string hostname, service, default_host("localhost");
    std::cout << "Enter a hostname (enter 'l' for localhost)" << std::endl;
    std::cin >> hostname;
    if (hostname == "l") {
        hostname = default_host;
    }
    std::cout << "Enter service / port: (this is a number)" << std::endl;
    std::cin >> service;
    c.connect_to_peer(hostname, service);
}

void get_peer_id_and_ping(Client &c) {
    peer_id id;
    std::cout << "Enter an ID: (it should start from 1, see the output above)"
              << std::endl;
    std::cin >> id;
    Message m(MessageType::PING);
    c.push_message(id, m);
}

void ping_all(Client &c) {
    Message m(MessageType::PING);
    c.broadcast(m);
}

void print_all_connected(Client &c) {
    std::cout << "===== LIST OF CONNECTIONS ======" << std::endl;
    std::error_code ec;
    for (auto s : c.get_sockets()) {
        auto endpoint = s.second->remote_endpoint(ec);
        if (ec) {
            std::cout << "ID: " << s.first << " | but it is down." << std::endl;
        }
        std::cout << "ID: " << s.first << " | IP: " << endpoint << std::endl;
    }
    std::cout << "===== ==== == =========== ======" << std::endl;
}

void get_track_info(Client &c) {
    GetTrackInfo g;
    std::cout << "Enter a track title to search" << std::endl;
    std::cin >> g.title;


    Message m(MessageType::GET_TRACK_INFO);
    m << g; // push the struct into the message body
    // ask for everybody about that track
    c.broadcast(m);
}

/**
 * semi-toy example - demonstrates the use of client class
 * the client needs to open a port on his computer, hence the argv[1]
 */
int main(int argc, char **argv) {
    // use in memory database for demonstration purposes
    Client cl(std::stoi(argv[1]), ":memory:");
    // add some dummy tracks to the database
    cl.populate_tracks();

    // simple console menu - it just pings the machines
    while (true) {
        std::cout << "Select one of the options\n"
                  << "A) Connect to an IP\n"
                  << "B) Ping a machine with peer_id\n"
                  << "C) Ping all machines\n"
                  << "D) Print all connected peers\n"
                  << "E) Ask for track"
                  << std::endl;
        char c;
        std::cin >> c;
        if (c == EOF) {
            std::cout << "Bye!" << std::endl;
            break;
        }
        if (!std::isalpha(c)) {
            std::cout << "I need letters! received: (" << c << ")" << std::endl;
            continue;
        }
        switch (std::toupper(c)) {
        case 'A':
            get_ip_and_connect(cl);
            break;
        case 'B':
            get_peer_id_and_ping(cl);
            break;
        case 'C':
            ping_all(cl);
            break;
        case 'D':
            print_all_connected(cl);
            break;
        case 'E':
            get_track_info(cl);
            break;
        default:
            std::cout << "Unknown option! Please try again" << std::endl;
        }
    }
}
