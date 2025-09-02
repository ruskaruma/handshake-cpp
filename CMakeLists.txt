cmake_minimum_required(VERSION 3.14)
project(handshake_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# -----------------------
# GoogleTest
# -----------------------
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# -----------------------
# Common sources
# -----------------------
set(COMMON_SOURCES
    src/frame.cpp
    src/log.cpp
    src/handshake.cpp
)

# -----------------------
# Server
# -----------------------
add_executable(server
    src/server.cpp
    src/server_impl.cpp
    ${COMMON_SOURCES}
)
target_include_directories(server PRIVATE include)

# -----------------------
# Client
# -----------------------
add_executable(client
    src/client.cpp
    ${COMMON_SOURCES}
)
target_include_directories(client PRIVATE include)

# -----------------------
# Tests
# -----------------------

# Frame tests
add_executable(test_frame
    tests/test_frame.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(test_frame gtest_main pthread)
target_include_directories(test_frame PRIVATE include)
add_test(NAME test_frame COMMAND test_frame)

# Handshake tests
add_executable(test_handshake
    tests/test_handshake.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(test_handshake gtest_main pthread)
target_include_directories(test_handshake PRIVATE include)
add_test(NAME test_handshake COMMAND test_handshake)

# Base64 + SHA1 tests
add_executable(test_base64_sha1
    tests/test_base64_sha1.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(test_base64_sha1 gtest_main pthread)
target_include_directories(test_base64_sha1 PRIVATE include)
add_test(NAME test_base64_sha1 COMMAND test_base64_sha1)

# Oversize payload tests
add_executable(test_oversize_payload
    tests/test_oversize_payload.cpp
    src/server_impl.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(test_oversize_payload gtest_main pthread)
target_include_directories(test_oversize_payload PRIVATE include)
add_test(NAME test_oversize_payload COMMAND test_oversize_payload)
