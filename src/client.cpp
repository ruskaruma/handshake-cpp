#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<string>
#include<vector>
#include "webserver/frame.hpp"
#include "webserver/base64.hpp"
#include "webserver/sha1.hpp"
static std::string make_key()
{
    return "dGhlIHNhbXBsZSBub25jZQ==";
}

int main(int argc,char** argv)
{
    const char* host="127.0.0.1";
    int port=8080;
    if(argc>=2) port=std::atoi(argv[1]);

    int fd=::socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)
    {
        perror("socket");
        return 1;
    }
    sockaddr_in addr{};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    inet_pton(AF_INET,host,&addr.sin_addr);
    if(::connect(fd,(sockaddr*)&addr,sizeof(addr))<0)
    {
        perror("connect");
        return 1;
    }
    std::string key=make_key();
    std::string req;
    req+="GET /chat HTTP/1.1\r\n";
    req+="Host: "+std::string(host)+":"+std::to_string(port)+"\r\n";
    req+="Upgrade: websocket\r\n";
    req+="Connection: Upgrade\r\n";
    req+="Sec-WebSocket-Key: "+key+"\r\n";
    req+="Sec-WebSocket-Version: 13\r\n";
    req+="\r\n";
    if(::send(fd,req.data(),req.size(),0)<0)
    {
        perror("send");
        return 1;
    }
    char buf[2048];
    ssize_t n=::recv(fd,buf,sizeof(buf)-1,0);
    if(n<=0)
    {
        perror("recv");
        return 1;
    }
    buf[n]=0;
    std::cout<<"handshake response:\n"<<buf<<"\n";
    webserver::Frame f{true,0x1,std::vector<uint8_t>{'h','e','l','l','o'}};
    auto out=webserver::build_frame(f);
    ::send(fd,out.data(),out.size(),0);
    uint8_t rbuf[4096];
    n=::recv(fd,rbuf,sizeof(rbuf),0);
    if(n>0)
    {
        webserver::Frame in;
        size_t used=0;
        if(webserver::parse_frame(rbuf,n,in,used))
        {
            if(in.opcode==0x1)
            {
                std::string msg(in.payload.begin(),in.payload.end());
                std::cout<<"got back: "<<msg<<"\n";
            }
        }
    }
    //sending ping
    webserver::Frame ping{true,0x9,std::vector<uint8_t>{'p','i','n','g'}};
    auto outp=webserver::build_frame(ping);
    ::send(fd,outp.data(),outp.size(),0);
    std::cout<<"sent ping frame\n";

    //pong receive(receiving)
    n=::recv(fd,rbuf,sizeof(rbuf),0);
    if(n>0)
    {
        webserver::Frame pong;
        size_t usedp=0;
        if(webserver::parse_frame(rbuf,n,pong,usedp))
        {
            if(pong.opcode==0xA)
            {
                std::string msg(pong.payload.begin(),pong.payload.end());
                std::cout<<"got pong: "<<msg<<"\n";
            }
        }
    }
    auto outc=webserver::build_close(1000,"normal closure");
    ::send(fd,outc.data(),outc.size(),0);
    std::cout<<"sent close frame\n";

    ::close(fd);
    return 0;
}
