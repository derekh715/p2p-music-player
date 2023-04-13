#include "client.h"
#include <thread>

int main(int argc, char **argv) {
    std::vector<std::tuple<std::string, std::string>> f;
    f.push_back(std::make_tuple("localhost", "8000"));
    Client c(4044, f);
    std::this_thread::sleep_for(5s);
    Message m(MessageType::PING);
    c.push_message(1, m);
}
