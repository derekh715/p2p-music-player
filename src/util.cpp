#include "util.h"
#include <algorithm>

void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

uintmax_t get_file_size(const std::filesystem::path &path) {
    // if the path really exists and it is a regular file
    // directory don't have sizes
    if (std::filesystem::exists(path) &&
        std::filesystem::is_regular_file(path)) {
        auto err = std::error_code{};
        uintmax_t filesize = std::filesystem::file_size(path, err);
        if (err || filesize != UINTMAX_MAX) {
            return filesize;
        }
    }

    return UINTMAX_MAX;
}

char num_to_hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                       '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

int hex_to_num[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                      0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};

std::string to_hex_string(uint8_t bytes[16]) {
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 16; i++) {
        // first four bits and last four bits
        ss << num_to_hex[bytes[i] >> 4] << num_to_hex[bytes[i] & 0xF];
    }
    return ss.str();
}
