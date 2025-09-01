# handshake-cpp

A minimal WebSocket server/client implementation in C++17 from scratch — no external libraries.

## Features
- Full WebSocket handshake (RFC 6455)
- Frame parsing & building (text, binary, ping, pong, close, continuation)
- Fragmented message support
- Multi-client support using `select()`
- Clean modular design (server, handshake, frame)

## Build

```bash
git clone https://github.com/<yourusername>/handshake-cpp.git
cd handshake-cpp
mkdir build && cd build
cmake ..
make -j4
