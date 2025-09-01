#include "webserver/server.hpp"
#include "webserver/http.hpp"
#include "webserver/sha1.hpp"
#include "webserver/base64.hpp"
#include "webserver/frame.hpp"
#include "webserver/log.hpp"

#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<unistd.h>
#include<iostream>
#include<cstring>

namespace webserver
{
    static const char* WS_GUID="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    WebSocketServer::WebSocketServer(int port):port_(port),lfd_(-1),fdmax_(-1)
    {
        FD_ZERO(&master_);
    }
    void WebSocketServer::run()
    {
        lfd_=::socket(AF_INET,SOCK_STREAM,0);
        if(lfd_<0){ perror("socket"); return; }
        int yes=1;
        ::setsockopt(lfd_,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
        sockaddr_in a{};
        a.sin_family=AF_INET;
        a.sin_addr.s_addr=htonl(INADDR_ANY);
        a.sin_port=htons(port_);
        if(::bind(lfd_,(sockaddr*)&a,sizeof(a))<0){ perror("bind"); return; }
        if(::listen(lfd_,16)<0){ perror("listen"); return; }
        log("listening on 0.0.0.0:"+std::to_string(port_));
        FD_SET(lfd_,&master_);
        fdmax_=lfd_;
        for(;;)
        {
            fd_set readfds=master_;
            if(::select(fdmax_+1,&readfds,nullptr,nullptr,nullptr)<0)
            {
                perror("select");
                break;
            }
            for(int fd=0;fd<=fdmax_;fd++)
            {
                if(FD_ISSET(fd,&readfds))
                {
                    if(fd==lfd_)
                    {
                        sockaddr_in cli{};
                        socklen_t cl=sizeof(cli);
                        int cfd=::accept(lfd_,(sockaddr*)&cli,&cl);
                        if(cfd<0){ perror("accept"); continue; }
                        FD_SET(cfd,&master_);
                        if(cfd>fdmax_) fdmax_=cfd;
                        handshaken_[cfd]=false;
                    }
                    else
                    {
                        if(!handshaken_[fd])
                        {
                            if(!perform_handshake(fd))
                            {
                                cleanup_client(fd);
                            }
                            else
                            {
                                handshaken_[fd]=true;
                                log("handshake complete; connection upgraded (fd="+std::to_string(fd)+")");
                                if(on_open) on_open(fd);
                            }
                        }
                        else
                        {
                            if(!handle_frame(fd))
                            {
                                cleanup_client(fd);
                            }
                        }
                    }
                }
            }
        }
    }
    void WebSocketServer::cleanup_client(int fd)
    {
        ::close(fd);
        FD_CLR(fd,&master_);
        handshaken_.erase(fd);
        frag_buffer_.erase(fd);
        frag_opcode_.erase(fd);
        log("connection closed (fd="+std::to_string(fd)+")");
        if(on_close) on_close(fd);
    }
    bool WebSocketServer::perform_handshake(int fd)
    {
        std::string rbuf; rbuf.reserve(4096);
        char tmp[2048];
        while(rbuf.find("\r\n\r\n")==std::string::npos)
        {
            ssize_t n=::recv(fd,tmp,sizeof(tmp),0);
            if(n<=0) return false;
            rbuf.append(tmp,tmp+n);
            if(rbuf.size()>32*1024) return false;
        }
        auto req=parse_http_request(rbuf);
        if(!req) return false;
        if(_lc(req->method)!="get" || _lc(req->version)!="http/1.1")
            return false;
        auto it=req->headers.find("sec-websocket-key");
        if(it==req->headers.end()) return false;
        std::string src=it->second;
        src+=WS_GUID;
        auto dig=sha1_bytes(src);
        auto acc=base64_encode(dig.data(),dig.size());
        std::string resp;
        resp+="HTTP/1.1 101 Switching Protocols\r\n";
        resp+="Upgrade: websocket\r\n";
        resp+="Connection: Upgrade\r\n";
        resp+="Sec-WebSocket-Accept: "+acc+"\r\n\r\n";
        if(::send(fd,resp.data(),resp.size(),0)<0) return false;
        return true;
    }
    bool WebSocketServer::handle_frame(int fd)
    {
        uint8_t buf[4096];
        ssize_t n=::recv(fd,buf,sizeof(buf),0);
        if(n<=0) return false;
        Frame f;
        size_t used=0;
        if(!parse_frame(buf,n,f,used))
        {
            log("protocol error parsing frame (fd="+std::to_string(fd)+")");
            auto out=build_close(1002,"protocol error");
            ::send(fd,out.data(),out.size(),0);
            return false;
        }
        if(f.opcode==OP_TEXT)
        {
            if(f.fin)
            {
                std::string msg(f.payload.begin(),f.payload.end());
                log("fd "+std::to_string(fd)+" said (text): "+msg);
                if(on_message) on_message(fd,f.payload,true);
                auto out=build_text(msg);
                ::send(fd,out.data(),out.size(),0);
            }
            else
            {
                frag_buffer_[fd]=f.payload;
                frag_opcode_[fd]=OP_TEXT;
            }
        }
        else if(f.opcode==OP_BINARY)
        {
            if(f.fin)
            {
                log("fd "+std::to_string(fd)+" sent binary of "+std::to_string(f.payload.size())+" bytes");
                if(on_message) on_message(fd,f.payload,false);
                auto out=build_binary(f.payload);
                ::send(fd,out.data(),out.size(),0);
            }
            else
            {
                frag_buffer_[fd]=f.payload;
                frag_opcode_[fd]=OP_BINARY;
            }
        }
        else if(f.opcode==OP_CONT)
        {
            auto& bufv=frag_buffer_[fd];
            bufv.insert(bufv.end(),f.payload.begin(),f.payload.end());
            if(f.fin)
            {
                uint8_t baseop=frag_opcode_[fd];
                if(baseop==OP_TEXT)
                {
                    std::string msg(bufv.begin(),bufv.end());
                    log("fd "+std::to_string(fd)+" (fragmented text): "+msg);
                    if(on_message) on_message(fd,bufv,true);
                    auto out=build_text(msg);
                    ::send(fd,out.data(),out.size(),0);
                }
                else if(baseop==OP_BINARY)
                {
                    log("fd "+std::to_string(fd)+" (fragmented binary, "+std::to_string(bufv.size())+" bytes)");
                    if(on_message) on_message(fd,bufv,false);
                    auto out=build_binary(bufv);
                    ::send(fd,out.data(),out.size(),0);
                }
                bufv.clear();
                frag_opcode_.erase(fd);
            }
        }
        else if(f.opcode==OP_PING)
        {
            log("ping received from fd "+std::to_string(fd)+", pong sent");
            auto out=build_pong(f);
            ::send(fd,out.data(),out.size(),0);
        }
        else if(f.opcode==OP_CLOSE)
        {
            log("close frame received (fd="+std::to_string(fd)+")");
            auto out=build_close(1000,"normal closure");
            ::send(fd,out.data(),out.size(),0);
            return false;
        }
        else
        {
            log("unknown opcode "+std::to_string(int(f.opcode))+" from fd "+std::to_string(fd));
            auto out=build_close(1002,"protocol error");
            ::send(fd,out.data(),out.size(),0);
            return false;
        }
        return true;
    }
}
