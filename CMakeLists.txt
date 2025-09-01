cmake_minimum_required(VERSION 3.10)
project(handshake-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

# Server executable
add_executable(server
    src/server.cpp
    src/server_impl.cpp
    src/handshake.cpp
    src/frame.cpp
    src/log.cpp
)

add_executable(client
    src/client.cpp
    src/frame.cpp
    src/log.cpp
)
