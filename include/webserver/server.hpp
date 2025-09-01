#pragma once
#include<functional>
#include<map>
#include<vector>
#include<string>
#include<cstdint>
namespace webserver
{
    class WebSocketServer
    {
    public:
        explicit WebSocketServer(int port);
        void run();
        std::function<void(int)> on_open;
        std::function<void(int,const std::vector<uint8_t>&,bool is_text)> on_message;
        std::function<void(int)> on_close;
    private:
        int port_;
        int lfd_;
        int fdmax_;
        fd_set master_;
        std::map<int,bool> handshaken_;
        std::map<int,std::vector<uint8_t>> frag_buffer_;
        std::map<int,uint8_t> frag_opcode_;
        bool perform_handshake(int fd);
        bool handle_frame(int fd);
        void cleanup_client(int fd);
    };
}
