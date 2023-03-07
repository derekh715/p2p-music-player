cmake_minimum_required(VERSION 3.2)
# project name, can be changed
project(csci3280-project CXX)

# gtkmm4 uses some feaetures in C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# magic
find_package(PkgConfig)

pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0)

# adding testing framework
include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG       v1.13.x
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# for CTest
enable_testing()

add_executable(test_lrc tests/test_lrc.cpp)
target_link_libraries(test_lrc GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_lrc)

# main executable
# add source files here
add_executable(p2pmss ./main.cpp)
target_link_libraries(p2pmss PRIVATE PkgConfig::GTKMM_VARS)
