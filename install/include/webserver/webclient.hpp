#pragma once
#include <string>
#include <vector>
#include <optional>   // ✅ fix added
#include "webserver/frame.hpp"

namespace webserver {

class WebSocketClient {
public:
    WebSocketClient();
    WebSocketClient(const std::string& host, int port, const std::string& path);
    ~WebSocketClient();

    bool connect(const std::string& host, int port, const std::string& path);

    bool send_text(const std::string& msg);
    bool send_binary(const std::vector<uint8_t>& data);
    bool send_ping(const std::vector<uint8_t>& data = {});
    std::optional<Frame> recv_message();   // ✅ ensure this is here

    void close();   // ✅ added earlier

private:
    int sockfd_;
    bool connected_;
};

} // namespace webserver
