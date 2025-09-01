#include "webserver/handshake.hpp"
#include "webserver/http.hpp"
#include "webserver/sha1.hpp"
#include "webserver/base64.hpp"
#include "webserver/log.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <optional>
namespace webserver
{
    static const char* WS_GUID="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    static bool _is_valid_upgrade(const HttpRequest& r)
    {
        if(_lc(r.method)!="get") return false;
        if(_lc(r.version)!="http/1.1") return false;
        auto up=r.headers.find("upgrade");
        auto co=r.headers.find("connection");
        auto ve=r.headers.find("sec-websocket-version");
        auto ky=r.headers.find("sec-websocket-key");
        if(up==r.headers.end() || !_token_has(up->second,"websocket"))
            return false;
        if(co==r.headers.end() || !_token_has(co->second,"upgrade"))
            return false;
        if(ve==r.headers.end() || _trim(ve->second)!="13")
            return false;
        if(ky==r.headers.end())
            return false;
        return true;
    }
    static std::optional<std::string> _accept_val(const HttpRequest& r)
    {
        auto it=r.headers.find("sec-websocket-key");
        if(it==r.headers.end()) return std::nullopt;
        std::string src=it->second;
        src+=WS_GUID;
        auto dig=sha1_bytes(src);
        return base64_encode(dig.data(),dig.size());
    }
    bool perform_handshake(int fd)
    {
        std::string rbuf; rbuf.reserve(4096);
        char tmp[2048];
        while(rbuf.find("\r\n\r\n")==std::string::npos)
        {
            ssize_t n=::recv(fd,tmp,sizeof(tmp),0);
            if(n<=0)
            {
                return false;
            }
            rbuf.append(tmp,tmp+n);
            if(rbuf.size()>32*1024)
            {
                return false;
            }
        }
        auto req=parse_http_request(rbuf);
        if(!req || !_is_valid_upgrade(*req))
        {
            static const char* bad="HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            ::send(fd,bad,std::strlen(bad),0);
            return false;
        }
        auto acc=_accept_val(*req);
        if(!acc)
        {
            static const char* bad="HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            ::send(fd,bad,std::strlen(bad),0);
            return false;
        }
        std::string resp;
        resp+="HTTP/1.1 101 Switching Protocols\r\n";
        resp+="Upgrade: websocket\r\n";
        resp+="Connection: Upgrade\r\n";
        resp+="Sec-WebSocket-Accept: "+*acc+"\r\n\r\n";
        if(::send(fd,resp.data(),resp.size(),0)<0)
        {
            return false;
        }
        log("handshake complete; connection upgraded (fd="+std::to_string(fd)+")");
        return true;
    }
}
