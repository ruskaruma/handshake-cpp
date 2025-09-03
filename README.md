# handshake-cpp

[![MIT License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)

**handshake-cpp** is a lightweight C++17 WebSocket library built from scratch, providing both server and client implementations. It follows the WebSocket protocol (RFC 6455) and is designed to be simple, modular, and ready to use as a standalone library.

## Features
- Complete WebSocket handshake (RFC 6455)
- Text, binary, ping/pong, and close frames
- Fragmented frame support with proper reassembly
- Robust error handling with appropriate close codes (1002, 1009)
- Configurable maximum payload size and connection backlog
- Unit tests with GoogleTest and GoogleMock for reliability
- Example echo client and server to demonstrate usage

## Quick Start

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
make test
sudo make install
```

### Usage

```cpp
#include <handshake-cpp/websocket_client.h>

int main() {
    handshake::WebSocketClient client("ws://localhost:8080");
    
    if (client.connect()) {
        client.send_text("Hello, WebSocket!");
        auto message = client.recv_message();
        client.close();
    }
    
    return 0;
}
```

### CMake Integration
```cmake
find_package(handshake-cpp REQUIRED)
target_link_libraries(your_target handshake-cpp::handshake-cpp)
```

## Examples

```bash
# Run echo server
./examples/echo_server --port 8080

# Run echo client
./examples/echo_client --url ws://localhost:8080
```

## Project Structure
```bash
        handshake-cpp/
        ├── CMakeLists.txt
        ├── LICENSE
        ├── README.md
        ├── .gitignore
        │
        ├── cmake/
        │   └── handshake-cpp-config.cmake.in
        │
        ├── include/
        │   └── webserver/
        │       ├── base64.hpp
        │       ├── frame.hpp
        │       ├── handshake.hpp
        │       ├── http.hpp
        │       ├── log.hpp
        │       ├── server.hpp
        │       ├── sha1.hpp
        │       └── webclient.hpp
        │
        ├── src/
        │   ├── client.cpp
        │   ├── frame.cpp
        │   ├── handshake.cpp
        │   ├── log.cpp
        │   ├── server.cpp
        │   ├── server_impl.cpp
        │   └── webclient.cpp
        │
        ├── tests/
        │   ├── test_base64_sha1.cpp
        │   ├── test_frame.cpp
        │   ├── test_handshake.cpp
        │   └── test_oversize_payload.cpp
        │
        ├── examples/
        │   └── echo_client.cpp
        │
        └── build/

```

## License
This project is licensed under the **MIT License**.
