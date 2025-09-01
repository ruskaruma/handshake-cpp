#include <gtest/gtest.h>
#include "webserver/base64.hpp"
#include "webserver/sha1.hpp"
TEST(Base64Test, EncodeDecodeSimple)
{
    std::vector<uint8_t> data={'h','i'};
    std::string enc=webserver::base64_encode(data);
    ASSERT_EQ(enc, "aGk=");  // "hi" in base64
}
TEST(Sha1Test, KnownDigest)
{
    std::string input="abc";
    auto digest=webserver::sha1_bytes(input);
    std::string hex;
    for (auto b : digest)
    {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", b);
        hex+=buf;
    }
    ASSERT_EQ(hex, "a9993e364706816aba3e25717850c26c9cd0d89d");
}
