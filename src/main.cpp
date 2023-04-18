// CSCI3280 Phase 1
// Thomas

#include "application.h"
#include <clocale>
#include <cstdlib>
#include <filesystem>
#include <locale>

int main(int argc, char **argv) {
    char lang[] = "LANG=c";
    putenv(lang);
    std::filesystem::path CurrentDirectory = std::filesystem::current_path();
    std::string PathEnvVariable = std::string("PATH=") +
                                  CurrentDirectory.string() + "\\bin;" +
                                  getenv("PATH");
    putenv(PathEnvVariable.data());
    // if we are using the same computer, then multiple instances
    // of the same program will write to the same database file
    // pass a database file so that they won't write to the same database
    // if nothing is passed, use sqlite.db
    std::string database_file;
    if (argc == 2) {
        database_file.assign(std::string(argv[1]));
    } else {
        database_file.assign(std::string("sqlite.db"));
    }
    auto application = MyApplication::create(database_file);
    return application->run(argc, argv);
}
