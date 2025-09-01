cmake_minimum_required(VERSION 3.10)
project(handshake-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Server executable
add_executable(server
    src/server.cpp
    src/server_impl.cpp
    src/handshake.cpp
    src/frame.cpp
    src/log.cpp
)

target_include_directories(server PRIVATE include)

# Client executable
add_executable(client
    src/client.cpp
    src/frame.cpp
    src/log.cpp
)

target_include_directories(client PRIVATE include)

# GoogleTest integration
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executables
add_executable(test_frame tests/test_frame.cpp src/frame.cpp src/log.cpp)
target_include_directories(test_frame PRIVATE include)
target_link_libraries(test_frame gtest_main)

add_executable(test_handshake tests/test_handshake.cpp src/handshake.cpp src/log.cpp)
target_include_directories(test_handshake PRIVATE include)
target_link_libraries(test_handshake gtest_main)

add_executable(test_base64_sha1 tests/test_base64_sha1.cpp src/log.cpp)
target_include_directories(test_base64_sha1 PRIVATE include)
target_link_libraries(test_base64_sha1 gtest_main)

include(GoogleTest)
gtest_discover_tests(test_frame)
gtest_discover_tests(test_handshake)
gtest_discover_tests(test_base64_sha1)
