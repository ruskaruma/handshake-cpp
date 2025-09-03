#pragma once
#include <vector>
#include <cstdint>
#include <map>
#include <functional>
#include <atomic>

namespace webserver
{
    struct Frame;

    class WebSocketServer
    {
    public:
        explicit WebSocketServer(int port);
        void run(std::atomic<bool>& stop);   // ✅ changed to atomic<bool>&
        void set_backlog(int backlog);
        void set_max_payload(int bytes);

        std::function<void(int)> on_open;
        std::function<void(int)> on_close;
        std::function<void(int,const std::vector<uint8_t>&,bool)> on_message;

    private:
        int port_;
        int lfd_;
        int fdmax_;
        int backlog_;
        int max_payload_;
        fd_set master_;
        std::map<int,bool> handshaken_;
        std::map<int,std::vector<uint8_t>> frag_buffer_;
        std::map<int,uint8_t> frag_opcode_;

        void cleanup_client(int fd);
        bool perform_handshake(int fd);
        bool handle_frame(int fd);
    };
}
