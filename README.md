# handshake-cpp

A WebSocket server and client implementation in C++17. Built to learn how WebSockets work internally following RFC 6455.

Started with just the handshake part and gradually added more features.

## Features

- WebSocket handshake (HTTP Upgrade)
- Text and binary message echo
- Frame parsing and building
- Ping/Pong support
- Fragmented message handling
- Multiple clients using select()
- Basic logging with timestamps

## Project Structure

```
handshake-cpp/
├── include/webserver/
│   ├── base64.hpp
│   ├── frame.hpp
│   ├── handshake.hpp
│   ├── http.hpp
│   ├── log.hpp
│   ├── server.hpp
│   └── sha1.hpp
├── src/
│   ├── client.cpp
│   ├── frame.cpp
│   ├── handshake.cpp
│   ├── log.cpp
│   ├── server.cpp
│   └── server_impl.cpp
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## Building

Requires CMake and C++17 compiler.

```bash
git clone https://github.com/yourusername/handshake-cpp.git
cd handshake-cpp
mkdir build && cd build
cmake ..
make
```

Builds two executables: `server` and `client`.

## Usage

Start server:
```bash
./server 8080
```

Run client:
```bash
./client 8080
```

The client connects and tests various features:
- WebSocket handshake
- Text message echo
- Binary message echo  
- Fragmented message reassembly
- Ping/Pong exchange
- Connection close

## Implementation Notes

Uses `select()` for handling multiple client connections. Basic implementation focused on understanding the protocol rather than performance or production use.