#include "client.h"
#include <thread>

int main(int argc, char **argv) {
    // argv 1: which port to listen
    // argv 2: which port our peer is?
    std::vector<std::tuple<std::string, std::string>> f;
    f.push_back(std::make_tuple("localhost", argv[2]));
    Client c(std::stoi(argv[1]), f);
    std::this_thread::sleep_for(5s);
    Message m(MessageType::PING);
    c.push_message(1, m);
}
