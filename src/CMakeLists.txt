cmake_minimum_required(VERSION 3.2)
# project name, can be changed
project(csci3280-project CXX)

# gtkmm4 uses some feaetures in C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# main executable
# add source files here
add_executable(p2pmss ./main.cpp)

# magic
find_package(PkgConfig)

pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)

target_link_libraries(p2pmss PRIVATE PkgConfig::GTKMM_VARS)
