#include "webserver/webclient.hpp"
#include "webserver/frame.hpp"
#include "webserver/log.hpp"
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<cstring>
#include<iostream>
namespace webserver
{
    WebSocketClient::WebSocketClient()
    : sockfd_(-1), connected_(false) {}
    WebSocketClient::WebSocketClient(const std::string& host, int port, const std::string& path)
    : sockfd_(-1), connected_(false)
    {
        connect(host, port, path);
    }
    WebSocketClient::~WebSocketClient()
    {
        if(connected_) 
        {
            close();
        }
}
bool WebSocketClient::connect(const std::string& host, int port, const std::string& path)
{
    struct hostent* server = gethostbyname(host.c_str());
    if(!server)
    {
        log("ERROR: no such host " + host);
        return false;
    }
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd_ < 0) 
    {
        perror("socket");
        return false;
    }
    sockaddr_in serv_addr{};
    serv_addr.sin_family=AF_INET;
    std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    if(::connect(sockfd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect");
        return false;
    }
    std::string handshake =
        "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + ":" + std::to_string(port) + "\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    if(::send(sockfd_, handshake.data(), handshake.size(), 0) < 0)
    {
        perror("send handshake");
        return false;
    }
    char buffer[1024];
    ssize_t n=::recv(sockfd_, buffer, sizeof(buffer) - 1, 0);
    if(n<=0)
    {
        perror("recv handshake");
        return false;
    }
    buffer[n] = '\0';
    std::string response(buffer);
    if (response.find("101 Switching Protocols") == std::string::npos)
    {
        log("Handshake failed: " + response);
        return false;
    }
    connected_=true;
    log("Client handshake complete with server " + host + ":" + std::to_string(port));
    return true;
}
bool WebSocketClient::send_text(const std::string& msg)
{
    if(!connected_)
    return false;
    auto out=build_text(msg, true);
    return ::send(sockfd_, out.data(), out.size(), 0) == (ssize_t)out.size();
}
bool WebSocketClient::send_binary(const std::vector<uint8_t>& data)
{
    if(!connected_)
    return false;
    auto out=build_binary(data, true);
    return ::send(sockfd_, out.data(), out.size(), 0) == (ssize_t)out.size();
}
bool WebSocketClient::send_ping(const std::vector<uint8_t>& data) {
    if(!connected_) return false;
    Frame f;
    f.fin=true;
    f.opcode=OP_PING;
    f.mask=true;
    f.payload=data;
    auto out=build_frame(f);
    return ::send(sockfd_, out.data(), out.size(), 0) == (ssize_t)out.size();
}
std::optional<Frame> WebSocketClient::recv_message()
{
    if(!connected_)
    return std::nullopt;
    uint8_t buf[4096];
    ssize_t n = ::recv(sockfd_, buf, sizeof(buf), 0);
    if(n<=0)
    {
        return std::nullopt;
    }
    Frame f;
    size_t used = 0;
    if(!parse_frame(buf, n, f, used))
    {
        log("protocol error while parsing frame in client");
        return std::nullopt;
    }
    return f;
}

void WebSocketClient::close()
{
    if(connected_)
    {
        auto out=build_close(1000, "normal closure");
        ::send(sockfd_, out.data(), out.size(), 0);
        ::shutdown(sockfd_, SHUT_RDWR);
        ::close(sockfd_);
        connected_=false;
        log("Client closed connection cleanly");
    }
}
}