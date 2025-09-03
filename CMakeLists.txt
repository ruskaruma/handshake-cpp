cmake_minimum_required(VERSION 3.10)
project(handshake-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library target
add_library(handshake-cpp
    src/server_impl.cpp
    src/frame.cpp
    src/handshake.cpp
    src/log.cpp
    src/webclient.cpp
)

target_include_directories(handshake-cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Server executable
add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE handshake-cpp)

# Client executable (old demo client)
add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE handshake-cpp)

# Echo client example
add_executable(echo_client examples/echo_client.cpp)
target_link_libraries(echo_client PRIVATE handshake-cpp)

# --------------------------
# GoogleTest integration
# --------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_frame tests/test_frame.cpp)
target_link_libraries(test_frame PRIVATE handshake-cpp gtest_main)

add_executable(test_handshake tests/test_handshake.cpp)
target_link_libraries(test_handshake PRIVATE handshake-cpp gtest_main)

add_executable(test_base64_sha1 tests/test_base64_sha1.cpp)
target_link_libraries(test_base64_sha1 PRIVATE handshake-cpp gtest_main)

add_executable(test_oversize_payload tests/test_oversize_payload.cpp)
target_link_libraries(test_oversize_payload PRIVATE handshake-cpp gtest_main)

include(GoogleTest)
gtest_discover_tests(test_frame)
gtest_discover_tests(test_handshake)
gtest_discover_tests(test_base64_sha1)
gtest_discover_tests(test_oversize_payload)

# --------------------------
# Installation
# --------------------------
include(CMakePackageConfigHelpers)

install(TARGETS handshake-cpp
    EXPORT handshake-cpp-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT handshake-cpp-targets
    NAMESPACE handshake-cpp::
    DESTINATION lib/cmake/handshake-cpp
)

configure_package_config_file(
    cmake/handshake-cpp-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/handshake-cpp-config.cmake
    INSTALL_DESTINATION lib/cmake/handshake-cpp
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/handshake-cpp-config-version.cmake
    VERSION 0.1.0
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/handshake-cpp-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/handshake-cpp-config-version.cmake
    DESTINATION lib/cmake/handshake-cpp
)
