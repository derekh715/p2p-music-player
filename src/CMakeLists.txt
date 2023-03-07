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
include(GoogleTest)

function(add_test target)
  add_executable(${target} ${ARGN})
  # each test needs these two libraries
  target_link_libraries(${target} GTest::gtest_main GTest::gmock)
  # let ctest acknowledge the existence of our target
  gtest_discover_tests(${target})
endfunction()

# add your tests here
# with the format: <target name> <...source files>
# for example, test_lrc.cpp needs lrc.cpp, so I list both of them here
# and this will compile into an executable called test_lrc
add_test(test_lrc tests/test_lrc.cpp lrc.cpp)

# main executable
# add source files here
# p2pmss will be the executable that has the main function
add_executable(p2pmss main.cpp lrc.cpp)
# we link p2pmss with the necessary libraries like gtkmm4
target_link_libraries(p2pmss PRIVATE PkgConfig::GTKMM_VARS)
