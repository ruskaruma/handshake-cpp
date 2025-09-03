#pragma once
#include<cstdint>
#include<vector>
#include<string>

namespace webserver
{
    enum Opcode : uint8_t
    {
        OP_CONT   = 0x0,
        OP_TEXT   = 0x1,
        OP_BINARY = 0x2,
        OP_CLOSE  = 0x8,
        OP_PING   = 0x9,
        OP_PONG   = 0xA
    };
    struct Frame
    {
        bool fin = true;
        uint8_t opcode = 0;
        std::vector<uint8_t> payload;
        bool mask = false;
        uint32_t masking_key = 0;
    };
    bool parse_frame(const uint8_t* data, size_t len, Frame& out, size_t& used);
    std::vector<uint8_t> build_frame(const Frame& f);
    std::vector<uint8_t> build_text(const std::string& msg, bool fin = true);
    std::vector<uint8_t> build_binary(const std::vector<uint8_t>& data, bool fin = true);
    std::vector<uint8_t> build_continuation(const std::vector<uint8_t>& data, bool fin = true);
    std::vector<uint8_t> build_close(uint16_t code, const std::string& reason);
    std::vector<uint8_t> build_ping(const std::vector<uint8_t>& payload = {});
    std::vector<uint8_t> build_pong(const Frame& ping);
}
