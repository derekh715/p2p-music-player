// CSCI3280 Phase 1
// Thomas

#include "listfiles.h"

boost::regex asciiRegex("[ -~]*");
boost::smatch smatch;

std::vector<std::filesystem::path> ListFiles::listfiles(const std::string& _dir, const std::vector<std::string>& ext, bool asciiOnly, bool recursive, bool verbose) {
    std::filesystem::path dir = std::filesystem::u8path(_dir);
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
        return {};
    if (verbose)
        std::cout << "Selected Directory Verified." << std::endl;

    std::vector<std::filesystem::path> listfiles(0);

    if (recursive) {
        for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(dir))
            if (std::filesystem::is_regular_file(entry.path()) && fit_ext(entry, ext) && (!asciiOnly || boost::regex_match(entry.path().string(), smatch, asciiRegex))) {
                listfiles.push_back(entry.path());
                if (verbose)
                    std::cout << entry.path() << std::endl;
            }
    }
    else {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dir))
            if (std::filesystem::is_regular_file(entry.path()) && fit_ext(entry, ext) && (!asciiOnly || boost::regex_match(entry.path().string(), smatch, asciiRegex))) {
                listfiles.push_back(entry.path());
                if (verbose)
                    std::cout << entry.path() << std::endl;
            }
    }
    if (verbose)
        std::cout << listfiles.size() << " files found" << std::endl;

    return listfiles;
}

bool ListFiles::fit_ext(const std::filesystem::directory_entry& entry, const std::vector<std::string>& ext) {
    for (const std::string _ext : ext)
        if (entry.path().extension().string() == _ext)
            return true;
    return false;
}