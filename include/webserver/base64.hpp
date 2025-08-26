#pragma once
#include<cstddef>
#include<cstdint>
#include<string>
#include<vector>

namespace webserver
{
    inline std::string base64_encode(const uint8_t* p,size_t n)
    {
        static const char* T="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve(((n+2)/3)*4);
        size_t i=0;
        while(i+3<=n)
        {
            uint32_t v=(uint32_t(p[i])<<16)|(uint32_t(p[i+1])<<8)|(uint32_t(p[i+2]));
            i+=3;
            out.push_back(T[(v>>18)&63]);
            out.push_back(T[(v>>12)&63]);
            out.push_back(T[(v>>6)&63]);
            out.push_back(T[v&63]);
        }
        if(i+1==n)
        {
            uint32_t v=(uint32_t(p[i])<<16);
            out.push_back(T[(v>>18)&63]);
            out.push_back(T[(v>>12)&63]);
            out.push_back('=');
            out.push_back('=');
        }
        else if(i+2==n)
        {
            uint32_t v=(uint32_t(p[i])<<16)|(uint32_t(p[i+1])<<8);
            out.push_back(T[(v>>18)&63]);
            out.push_back(T[(v>>12)&63]);
            out.push_back(T[(v>>6)&63]);
            out.push_back('=');
        }
        return out;
    }
    inline std::string base64_encode(const std::vector<uint8_t>& v)
    {
        return base64_encode(v.data(),v.size());
    }
}
