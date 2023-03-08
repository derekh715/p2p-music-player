#include "lrc.h"
#include <iostream>

int main(int argc, char **argv) {
    Lrc f("./src/tests/data/jamaica_farewell.lrc");
    for (auto &s: f.getAllLyrics()) {
        std::cout << s << std::endl;
    }
    std::cout << "It works for the moment" << std::endl;
    return 0;
}
