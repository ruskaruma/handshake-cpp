#include "webserver/server.hpp"
#include "webserver/frame.hpp"
#include "webserver/log.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

// Helper: read close frame and extract code
static uint16_t read_close_code(int fd) {
    uint8_t buf[4096];
    ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
    if (n <= 0) return 0;

    webserver::Frame f;
    size_t used = 0;
    if (!webserver::parse_frame(buf, n, f, used)) {
        return 0;
    }
    if (f.opcode != webserver::OP_CLOSE) return 0;
    if (f.payload.size() < 2) return 0;

    return (uint16_t(f.payload[0]) << 8) | f.payload[1];
}

class OversizePayloadTest : public ::testing::Test {};

TEST_F(OversizePayloadTest, TextMessageTooBig) {
    std::atomic<bool> stop(false);
    webserver::WebSocketServer server(9091);
    server.set_max_payload(65536); // 64 KB max

    std::thread t([&]() { server.run(stop); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // connect as client
    int cfd = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(cfd, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9091);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ASSERT_EQ(::connect(cfd, (sockaddr*)&addr, sizeof(addr)), 0);

    // Perform minimal handshake
    std::string req =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    ::send(cfd, req.data(), req.size(), 0);
    char resp[4096];
    ssize_t n = ::recv(cfd, resp, sizeof(resp), 0);
    ASSERT_GT(n, 0);

    // Build oversize text message ( >64KB )
    std::string bigmsg(70000, 'A');
    auto frame = webserver::build_text(bigmsg);
    ::send(cfd, frame.data(), frame.size(), 0);

    // Read close frame
    uint16_t code = read_close_code(cfd);
    EXPECT_EQ(code, 1009); // Message Too Big

    ::close(cfd);
    stop = true;
    t.join();
}

TEST_F(OversizePayloadTest, BinaryMessageTooBig) {
    std::atomic<bool> stop(false);
    webserver::WebSocketServer server(9092);
    server.set_max_payload(65536); // 64 KB max

    std::thread t([&]() { server.run(stop); });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // connect as client
    int cfd = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(cfd, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9092);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ASSERT_EQ(::connect(cfd, (sockaddr*)&addr, sizeof(addr)), 0);

    // Perform minimal handshake
    std::string req =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    ::send(cfd, req.data(), req.size(), 0);
    char resp[4096];
    ssize_t n = ::recv(cfd, resp, sizeof(resp), 0);
    ASSERT_GT(n, 0);

    // Build oversize binary message ( >64KB )
    std::vector<uint8_t> bigbin(70000, 0x42);
    auto frame = webserver::build_binary(bigbin);
    ::send(cfd, frame.data(), frame.size(), 0);

    // Read close frame
    uint16_t code = read_close_code(cfd);
    EXPECT_EQ(code, 1009); // Message Too Big

    ::close(cfd);
    stop = true;
    t.join();
}
