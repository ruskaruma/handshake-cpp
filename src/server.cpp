#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<optional>
#include<string>
#include<vector>
#include<map>
#include "webserver/http.hpp"
#include "webserver/sha1.hpp"
#include "webserver/base64.hpp"
#include "webserver/frame.hpp"
namespace webserver
{
    static const char* WS_GUID="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    static bool _is_valid_upgrade(const HttpRequest& r)
    {
        if(_lc(r.method)!="get")
        return false;
        if(_lc(r.version)!="http/1.1")
        return false;
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
        if(it==r.headers.end())
        return std::nullopt;
        std::string src=it->second;
        src+=WS_GUID;
        auto dig=sha1_bytes(src);
        return base64_encode(dig.data(),dig.size());
    }
    static std::string _resp101(const std::string& acc)
    {
        std::string out;
        out.reserve(128);
        out+="HTTP/1.1 101 Switching Protocols\r\n";
        out+="Upgrade: websocket\r\n";
        out+="Connection: Upgrade\r\n";
        out+="Sec-WebSocket-Accept: "+acc+"\r\n";
        out+="\r\n";
        return out;
    }
    static bool perform_handshake(int fd)
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
        auto resp=_resp101(*acc);
        if(::send(fd,resp.data(),resp.size(),0)<0)
        {
            return false;
        }
        std::cout<<"handshake complete; connection upgraded (fd="<<fd<<")\n";
        return true;
    }
    static bool handle_frame(int fd)
    {
        uint8_t buf[4096];
        ssize_t n=::recv(fd,buf,sizeof(buf),0);
        if(n<=0)
        {
            return false; //if closed or error
        }
        Frame f;
        size_t used=0;
        if(!parse_frame(buf,n,f,used))
        {
            std::cerr<<"frame parse failed\n";
            return false;
        }
        if(f.opcode==0x1) 
        {
            std::string msg(f.payload.begin(),f.payload.end());
            std::cout<<"fd "<<fd<<" said: "<<msg<<"\n";

            Frame reply{true,0x1,std::vector<uint8_t>(msg.begin(),msg.end())};
            auto out=build_frame(reply);
            ::send(fd,out.data(),out.size(),0);
        }
        else if(f.opcode==0x9) 
        {
            auto out=build_pong(f);
            ::send(fd,out.data(),out.size(),0);
            std::cout<<"ping received from fd "<<fd<<", pong sent\n";
        }
        else if(f.opcode==0x8) 
        {
            auto out=build_close(1000,"normal closure");
            ::send(fd,out.data(),out.size(),0);
            std::cout<<"connection closed (fd="<<fd<<")\n";
            return false;
        }
        return true;
    }
}
int main(int argc,char** argv)
{
    int port=8080;
    if(argc>=2) port=std::atoi(argv[1]);

    int lfd=::socket(AF_INET,SOCK_STREAM,0);
    if(lfd<0)
    {
        perror("socket");
        return 1;
    }
    int yes=1;
    ::setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    sockaddr_in a{};
    a.sin_family=AF_INET;
    a.sin_addr.s_addr=htonl(INADDR_ANY);
    a.sin_port=htons(port);
    if(::bind(lfd,(sockaddr*)&a,sizeof(a))<0)
    {
        perror("bind");
        return 1;
    }
    if(::listen(lfd,16)<0)
    {
        perror("listen");
        return 1;
    }
    std::cout<<"listening on 0.0.0.0:"<<port<<"\n";
    fd_set master,readfds;
    FD_ZERO(&master);
    FD_SET(lfd,&master);
    int fdmax=lfd;
    std::map<int,bool> handshaken; // track which fds finished handshake
    for(;;)
    {
        readfds=master;
        if(::select(fdmax+1,&readfds,nullptr,nullptr,nullptr)<0)
        {
            perror("select");
            break;
        }
        for(int fd=0;fd<=fdmax;fd++)
        {
            if(FD_ISSET(fd,&readfds))
            {
                if(fd==lfd)
                {
                    // new connection
                    sockaddr_in cli{};
                    socklen_t cl=sizeof(cli);
                    int cfd=::accept(lfd,(sockaddr*)&cli,&cl);
                    if(cfd<0)
                    {
                        perror("accept");
                        continue;
                    }
                    FD_SET(cfd,&master);
                    if(cfd>fdmax) fdmax=cfd;
                    handshaken[cfd]=false;
                }
                else
                {
                    if(!handshaken[fd])
                    {
                        if(!webserver::perform_handshake(fd))
                        {
                            ::close(fd);
                            FD_CLR(fd,&master);
                            handshaken.erase(fd);
                        }
                        else
                        {
                            handshaken[fd]=true;
                        }
                    }
                    else
                    {
                        if(!webserver::handle_frame(fd))
                        {
                            ::close(fd);
                            FD_CLR(fd,&master);
                            handshaken.erase(fd);
                        }
                    }
                }
            }
        }
    }
    ::close(lfd);
    return 0;
}
