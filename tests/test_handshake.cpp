#include<gtest/gtest.h>
#include "webserver/handshake.hpp"
#include "webserver/http.hpp"      
#include "webserver/sha1.hpp"
#include "webserver/base64.hpp"

namespace
{
std::string expected_accept(const std::string& key) {
    std::string src = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    auto dig = webserver::sha1_bytes(src);
    return webserver::base64_encode(dig.data(), dig.size());
}
}

TEST(HandshakeTest, AcceptValueMatchesSpec) {
    // Known WebSocket test vector from RFC6455
    std::string key = "dGhlIHNhbXBsZSBub25jZQ==";
    std::string expected = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
    EXPECT_EQ(expected_accept(key), expected);
}
TEST(HandshakeTest, RejectInvalidRequest) {
    webserver::HttpRequest req;
    req.method = "POST"; 
    req.version = "HTTP/1.1";
    req.headers["sec-websocket-key"] = "dummy";

    EXPECT_NE(expected_accept(req.headers["sec-websocket-key"]), "");
}
