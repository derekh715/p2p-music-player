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
    auto application = MyApplication::create();
    return application->run(argc, argv);
}
