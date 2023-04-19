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
    // now also accepts a port argument for local testing
    std::string database_file = "sqlite.db";
    uint16_t port = 4000;
    // if there are three arguments, that means the first one is db file name
    // the second one is port
    if (argc == 3) {
        database_file.assign(argv[1]);
        port = std::atoi(argv[2]);
        // if not there is only db filename
    } else if (argc == 2) {
        database_file.assign(argv[1]);
    }
    auto application = MyApplication::create(database_file, port);

    // apparently gtk hates command line arguments? Why?
    if (argc == 3) {
        return application->run(argc - 2, argv);
    } else if (argc == 2) {
        return application->run(argc - 1, argv);
    }

    return application->run(argc, argv);
}
