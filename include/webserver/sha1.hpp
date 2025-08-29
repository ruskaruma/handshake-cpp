#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <string>

namespace webserver
{
    // minimal SHA-1 (sufficient for handshake)
    class Sha1
    {
    public:
        Sha1()
        : h0_(0x67452301), h1_(0xEFCDAB89), h2_(0x98BADCFE), h3_(0x10325476), h4_(0xC3D2E1F0),
          bits_(0), buf_() {}

        void update(const void* p, size_t n)
        {
            const uint8_t* s = static_cast<const uint8_t*>(p);
            bits_ += uint64_t(n) * 8;
            buf_.insert(buf_.end(), s, s + n);
            while (buf_.size() >= 64) { _block(&buf_[0]); buf_.erase(buf_.begin(), buf_.begin() + 64); }
        }

        std::array<uint8_t,20> finalize()
        {
            std::vector<uint8_t> t = buf_;
            t.push_back(0x80);
            while ((t.size() % 64) != 56) t.push_back(0x00);
            for (int i = 7; i >= 0; --i) t.push_back(uint8_t((bits_ >> (i*8)) & 0xFF));
            for (size_t off = 0; off < t.size(); off += 64) _block(&t[off]);

            std::array<uint8_t,20> out{};
            _put32(out, 0, h0_); _put32(out, 4, h1_); _put32(out, 8, h2_);
            _put32(out,12, h3_); _put32(out,16, h4_);
            return out;
        }

    private:
        uint32_t h0_, h1_, h2_, h3_, h4_;
        uint64_t bits_;
        std::vector<uint8_t> buf_;

        static inline uint32_t _rol(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }
        static inline void _put32(std::array<uint8_t,20>& o, int idx, uint32_t v)
        {
            o[idx+0] = uint8_t((v >> 24) & 0xFF);
            o[idx+1] = uint8_t((v >> 16) & 0xFF);
            o[idx+2] = uint8_t((v >>  8) & 0xFF);
            o[idx+3] = uint8_t((v      ) & 0xFF);
        }

        void _block(const uint8_t* b)
        {
            uint32_t w[80];
            for (int i = 0; i < 16; ++i)
                w[i] = (uint32_t(b[i*4+0]) << 24) | (uint32_t(b[i*4+1]) << 16)
                     | (uint32_t(b[i*4+2]) << 8)  | (uint32_t(b[i*4+3]));
            for (int i = 16; i < 80; ++i) { uint32_t v = w[i-3]^w[i-8]^w[i-14]^w[i-16]; w[i] = (v << 1) | (v >> 31); }

            uint32_t a = h0_, bv = h1_, c = h2_, d = h3_, e = h4_;
            for (int i = 0; i < 80; ++i)
            {
                uint32_t f, k;
                if (i < 20)      { f = (bv & c) | ((~bv) & d); k = 0x5A827999; }
                else if (i < 40) { f = bv ^ c ^ d;             k = 0x6ED9EBA1; }
                else if (i < 60) { f = (bv & c) | (bv & d) | (c & d); k = 0x8F1BBCDC; }
                else             { f = bv ^ c ^ d;             k = 0xCA62C1D6; }
                uint32_t tmp = _rol(a,5) + f + e + k + w[i];
                e = d; d = c; c = _rol(bv,30); bv = a; a = tmp;
            }
            h0_ += a; h1_ += bv; h2_ += c; h3_ += d; h4_ += e;
        }
    };

    inline std::array<uint8_t,20> sha1_bytes(const std::string& s)
    {
        Sha1 sh; sh.update(s.data(), s.size()); return sh.finalize();
    }
}
