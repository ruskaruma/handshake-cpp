#include "webserver/frame.hpp"
#include <cstring>
namespace webserver
{
    bool parse_frame(const uint8_t* buf, size_t n, Frame& out, size_t& used)
    {
        if(n<2) return false;
        uint8_t b0=buf[0];
        uint8_t b1=buf[1];
        out.fin=(b0 & 0x80)!=0;
        out.opcode=(b0 & 0x0F);
        bool masked=(b1 & 0x80)!=0;
        uint64_t len=(b1 & 0x7F);
        size_t pos=2;
        if(len==126)
        {
            if(n<pos+2) return false;
            len=(uint64_t(buf[pos])<<8)|buf[pos+1];
            pos+=2;
        }
        else if(len==127)
        {
            if(n<pos+8) return false;
            len=0;
            for(int i=0;i<8;i++) len=(len<<8)|buf[pos+i];
            pos+=8;
        }
        uint8_t mask[4];
        if(masked)
        {
            if(n<pos+4) return false;
            for(int i=0;i<4;i++) mask[i]=buf[pos+i];
            pos+=4;
        }
        if(n<pos+len) return false;
        out.payload.resize(len);
        for(size_t i=0;i<len;i++)
        {
            uint8_t c=buf[pos+i];
            if(masked) c^=mask[i%4];
            out.payload[i]=c;
        }
        pos+=len;
        used=pos;
        return true;
    }
    std::vector<uint8_t> build_frame(const Frame& f)
    {
        std::vector<uint8_t> out;
        uint8_t b0=(f.fin?0x80:0x00)|(f.opcode&0x0F);
        out.push_back(b0);
        size_t len=f.payload.size();
        if(len<126)
        {
            out.push_back(uint8_t(len));
        }
        else if(len<=0xFFFF)
        {
            out.push_back(126);
            out.push_back((len>>8)&0xFF);
            out.push_back(len&0xFF);
        }
        else
        {
            out.push_back(127);
            for(int i=7;i>=0;i--)
                out.push_back((len>>(i*8))&0xFF);
        }

        out.insert(out.end(), f.payload.begin(), f.payload.end());
        return out;
    }
    std::vector<uint8_t> build_pong(const Frame& ping)
    {
        Frame f{true,OP_PONG,ping.payload};
        return build_frame(f);
    }
    std::vector<uint8_t> build_close(uint16_t code,const std::string& reason)
    {
        std::vector<uint8_t> payload;
        payload.push_back((code>>8)&0xFF);
        payload.push_back(code&0xFF);
        payload.insert(payload.end(),reason.begin(),reason.end());
        Frame f{true,OP_CLOSE,payload};
        return build_frame(f);
    }
    std::vector<uint8_t> build_text(const std::string& msg,bool fin)
    {
        Frame f{fin,OP_TEXT,std::vector<uint8_t>(msg.begin(),msg.end())};
        return build_frame(f);
    }
    std::vector<uint8_t> build_binary(const std::vector<uint8_t>& data,bool fin)
    {
        Frame f{fin,OP_BINARY,data};
        return build_frame(f);
    }
    std::vector<uint8_t> build_continuation(const std::vector<uint8_t>& data,bool fin)
    {
        Frame f{fin,OP_CONT,data};
        return build_frame(f);
    }
}
