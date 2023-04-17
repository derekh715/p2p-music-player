cmake_minimum_required(VERSION 3.2)
# project name, can be changed
project(csci3280-project CXX)

# see which configuration we are in
message( STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}" )

# modern cpp features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set extra flags here if you need
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# magic
find_package(PkgConfig)

pkg_check_modules(gtk3 REQUIRED IMPORTED_TARGET gtkmm-3.0)
pkg_check_modules(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.4)
pkg_check_modules(gstreamer-sdp REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.4)
pkg_check_modules(gstreamer-app REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.4)
pkg_check_modules(gstreamer-video REQUIRED IMPORTED_TARGET gstreamer-video-1.0>=1.4)
pkg_check_modules(gstreamer-audio REQUIRED IMPORTED_TARGET gstreamer-audio-1.0>=1.4)
pkg_check_modules(taglib REQUIRED IMPORTED_TARGET taglib)
pkg_check_modules(asio REQUIRED IMPORTED_TARGET asio)

# pull those libraries here and build them
include(FetchContent)

FetchContent_Declare(SQLiteCpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp
)

# adding testing framework
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG       v1.13.x
)

# sqlitcpp library
FetchContent_MakeAvailable(SQLiteCpp)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# for CTest
enable_testing()
include(GoogleTest)

function(add_test target libs)
  add_executable(${target} ${ARGN})
  # each test needs these two libraries
  target_link_libraries(${target} GTest::gtest_main GTest::gmock ${libs})
  # let ctest acknowledge the existence of our target
  gtest_discover_tests(${target})
endfunction()

# add your tests here
# with the format: <target name> <...source files>
# for example, test_lrc.cpp needs lrc.cpp, so I list both of them here
# and this will compile into an executable called test_lrc
add_test(test_lrc "" tests/test_lrc.cpp util.cpp lrc.cpp)
add_test(test_db SQLiteCpp tests/test_db.cpp store.cpp store-types.cpp util.cpp
  md5.cpp)
add_test(test_queue "" tests/test_tsqueue.cpp)
add_test(test_msg "" tests/test_msg.cpp message.cpp store-types.cpp lrc.cpp
  util.cpp)
add_test(test_chunk "" tests/test_chunk.cpp chunked-file.cpp util.cpp)

set(base_srcs util.cpp store.cpp client.cpp base-client.cpp message.cpp
  store-types.cpp store-types.cpp lrc.cpp md5.cpp chunked-file.cpp)

# main executable
# add source files here
# p2pmss will be the executable that has the main function
add_executable(p2pmss main.cpp application.cpp
  wav.cpp listfiles.cpp ${base_srcs})
add_executable(network network-example.cpp ${base_srcs})
# we link p2pmss with the necessary libraries like gtkmm4
target_link_libraries(p2pmss PRIVATE
  SQLiteCpp
  PkgConfig::gtk3
  PkgConfig::gstreamer
  PkgConfig::gstreamer-sdp
  PkgConfig::gstreamer-app
  PkgConfig::gstreamer-video
  PkgConfig::gstreamer-audio
  PkgConfig::taglib
  PkgConfig::asio
)
target_link_libraries(network PRIVATE PkgConfig::asio SQLiteCpp)
