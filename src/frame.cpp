#include "webserver/frame.hpp"
#include <cstring>
#include <arpa/inet.h>

namespace webserver
{
    bool parse_frame(const uint8_t* data,size_t len,Frame& frame,size_t& used)
    {
        if(len<2) return false;
        uint8_t b0=data[0];
        uint8_t b1=data[1];
        bool mask=(b1&0x80)!=0;
        uint64_t plen=b1&0x7F;
        size_t off=2;
        if(plen==126)
        {
            if(len<4) return false;
            uint16_t v; std::memcpy(&v,data+2,2);
            plen=ntohs(v);
            off=4;
        }
        else if(plen==127)
        {
            if(len<10) return false;
            uint64_t v; std::memcpy(&v,data+2,8);
            plen=be64toh(v);
            off=10;
        }
        uint8_t maskkey[4]{};
        if(mask)
        {
            if(len<off+4) return false;
            std::memcpy(maskkey,data+off,4);
            off+=4;
        }

        // --- Oversize patch ---
        if(plen>65536)   // >64KB → let server handle with 1009
        {
            frame.fin=(b0&0x80)!=0;
            frame.opcode=b0&0x0F;
            frame.payload.resize(plen,0);  // just reserve size
            used=len; // consume buffer
            return true;
        }
        // ----------------------

        if(len<off+plen) return false;
        frame.fin=(b0&0x80)!=0;
        frame.opcode=b0&0x0F;
        frame.payload.resize(plen);
        if(mask)
        {
            for(size_t i=0;i<plen;i++)
                frame.payload[i]=data[off+i]^maskkey[i%4];
        }
        else
        {
            std::memcpy(frame.payload.data(),data+off,plen);
        }
        used=off+plen;
        return true;
    }

    std::vector<uint8_t> build_frame(const Frame& f)
    {
        std::vector<uint8_t> out;
        uint8_t b0=(f.fin?0x80:0x00)|(f.opcode&0x0F);
        out.push_back(b0);
        size_t plen=f.payload.size();
        if(plen<=125)
        {
            out.push_back(uint8_t(plen));
        }
        else if(plen<=0xFFFF)
        {
            out.push_back(126);
            uint16_t v=htons(uint16_t(plen));
            uint8_t* p=(uint8_t*)&v;
            out.insert(out.end(),p,p+2);
        }
        else
        {
            out.push_back(127);
            uint64_t v=htobe64(plen);
            uint8_t* p=(uint8_t*)&v;
            out.insert(out.end(),p,p+8);
        }
        out.insert(out.end(),f.payload.begin(),f.payload.end());
        return out;
    }

    std::vector<uint8_t> build_text(const std::string& msg,bool fin)
    {
        Frame f;
        f.fin=fin;
        f.opcode=OP_TEXT;
        f.payload.assign(msg.begin(),msg.end());
        return build_frame(f);
    }

    std::vector<uint8_t> build_binary(const std::vector<uint8_t>& data,bool fin)
    {
        Frame f;
        f.fin=fin;
        f.opcode=OP_BINARY;
        f.payload=data;
        return build_frame(f);
    }

    std::vector<uint8_t> build_continuation(const std::vector<uint8_t>& data,bool fin)
    {
        Frame f;
        f.fin=fin;
        f.opcode=OP_CONT;
        f.payload=data;
        return build_frame(f);
    }

    std::vector<uint8_t> build_close(uint16_t code,const std::string& reason)
    {
        Frame f;
        f.fin=true;
        f.opcode=OP_CLOSE;
        f.payload.resize(2+reason.size());
        uint16_t v=htons(code);
        std::memcpy(f.payload.data(),&v,2);
        std::memcpy(f.payload.data()+2,reason.data(),reason.size());
        return build_frame(f);
    }

    std::vector<uint8_t> build_pong(const Frame& ping)
    {
        Frame f;
        f.fin=true;
        f.opcode=OP_PONG;
        f.payload=ping.payload;
        return build_frame(f);
    }
}
