// CSCI3280 Phase 1
// Thomas

#ifndef MYLISTFILES_H
#define MYLISTFILES_H

#include <filesystem>
#include <vector>
#include <string>
#include <boost/regex.hpp>
#include <iostream>

class ListFiles {
public:
    static std::vector<std::filesystem::path> listfiles(const std::string& _dir, const std::vector<std::string>& ext = {}, bool asciiOnly = true, bool recursive = false, bool verbose = false);

private:
    static bool fit_ext(const std::filesystem::directory_entry& entry, const std::vector<std::string>& ext);
};

#endif /* MYLISTFILES_H */