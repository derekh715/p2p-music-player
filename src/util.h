#ifndef UTIL_H__
#define UTIL_H__
#include <filesystem>
#include <string>

typedef uint16_t peer_id;

void rtrim(std::string &s);

uintmax_t get_file_size(const std::filesystem::path &path);

std::string to_hex_string(uint8_t bytes[16]);

#endif
