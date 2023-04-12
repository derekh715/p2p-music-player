#include "client.h"

int main(int argc, char **argv) {
    std::list<std::tuple<std::string, std::string>> f;
    f.push_back(std::make_tuple("localhost", "8000"));
    Client c(4044, f);
}
