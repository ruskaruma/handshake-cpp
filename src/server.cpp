#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include "webserver/http.hpp"
#include "webserver/sha1.hpp"
#include "webserver/base64.hpp"

namespace webserver
{
    static const char* WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    static bool _is_valid_upgrade(const HttpRequest& r)
    {
        if (_lc(r.method)  != "get")      return false;
        if (_lc(r.version) != "http/1.1") return false;

        auto up = r.headers.find("upgrade");
        auto co = r.headers.find("connection");
        auto ve = r.headers.find("sec-websocket-version");
        auto ky = r.headers.find("sec-websocket-key");

        if (up == r.headers.end() || !_token_has(up->second, "websocket")) return false;
        if (co == r.headers.end() || !_token_has(co->second, "upgrade"))   return false;
        if (ve == r.headers.end() || _trim(ve->second) != "13")            return false;
        if (ky == r.headers.end())                                         return false;
        return true;
    }

    static std::optional<std::string> _accept_val(const HttpRequest& r)
    {
        auto it = r.headers.find("sec-websocket-key");
        if (it == r.headers.end()) return std::nullopt;

        std::string src = it->second; src += WS_GUID;
        auto dig = sha1_bytes(src);
        return base64_encode(dig.data(), dig.size());
    }

    static std::string _resp101(const std::string& acc)
    {
        std::string out;
        out.reserve(128);
        out += "HTTP/1.1 101 Switching Protocols\r\n";
        out += "Upgrade: websocket\r\n";
        out += "Connection: Upgrade\r\n";
        out += "Sec-WebSocket-Accept: " + acc + "\r\n";
        out += "\r\n";
        return out;
    }
}

int main(int argc, char** argv)
{
    int port = 8080;
    if (argc >= 2) port = std::atoi(argv[1]);

    int lfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) { perror("socket"); return 1; }

    int yes = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port = htons(port);

    if (bind(lfd, (sockaddr*)&a, sizeof(a)) < 0) { perror("bind"); return 1; }
    if (listen(lfd, 16) < 0) { perror("listen"); return 1; }

    std::cout << "listening on 0.0.0.0:" << port << "\n";

    for (;;)
    {
        sockaddr_in cli{}; socklen_t cl = sizeof(cli);
        int fd = accept(lfd, (sockaddr*)&cli, &cl);
        if (fd < 0) { perror("accept"); continue; }

        std::string rbuf; rbuf.reserve(4096);
        char tmp[2048];

        // read until \r\n\r\n
        while (rbuf.find("\r\n\r\n") == std::string::npos)
        {
            ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
            if (n <= 0) { if (n < 0) perror("recv"); close(fd); goto next; }
            rbuf.append(tmp, tmp + n);
            if (rbuf.size() > 32 * 1024) { std::cerr << "headers too large\n"; close(fd); goto next; }
        }

        auto req = webserver::parse_http_request(rbuf);
        if (!req || !webserver::_is_valid_upgrade(*req))
        {
            static const char* bad = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            send(fd, bad, std::strlen(bad), 0);
            close(fd);
            goto next;
        }

        auto acc = webserver::_accept_val(*req);
        if (!acc)
        {
            static const char* bad = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            send(fd, bad, std::strlen(bad), 0);
            close(fd);
            goto next;
        }

        {
            auto resp = webserver::_resp101(*acc);
            if (send(fd, resp.data(), resp.size(), 0) < 0) { perror("send"); close(fd); goto next; }
        }

        std::cout << "handshake complete; connection upgraded\n";

        // Day 1: no frame handling yet
        sleep(5);
        close(fd);

    next:
        ;
    }

    close(lfd);
    return 0;
}
