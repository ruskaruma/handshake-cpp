# handshake-cpp

A minimal WebSocket implementation in C++ built from scratch.  
The project starts with the WebSocket handshake (HTTP Upgrade) and will grow into full frame parsing and message handling.

## Objectives
- Implement WebSocket HTTP upgrade handshake
- Parse and validate HTTP headers
- Generate `Sec-WebSocket-Accept` correctly using SHA-1 + Base64
- Establish foundation for frame parsing and message handling

## Build
```bash
g++ -std=c++17 -Wall -Wextra -o server server.cpp
./server
