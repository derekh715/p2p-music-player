// CSCI3280 Phase 1
// Thomas

#include <cstdlib>
#include <stdlib.h>  
#include "application.h"
#include <gst/gst.h>
#include <iostream>
#include <locale>
#include <clocale>
#include <filesystem>

int main(int argc, char* argv[])
{
  putenv("LANG=c");
  std::filesystem::path CurrentDirectory = std::filesystem::current_path();
  std::string PathEnvVariable = std::string("PATH=") + CurrentDirectory.string() + "\\bin;" + getenv("PATH");
  putenv(PathEnvVariable.c_str());

  auto application = MyApplication::create();
  return application->run(argc, argv);
}
