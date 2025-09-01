#include<gtest/gtest.h>
#include "webserver/frame.hpp"
TEST(FrameTest, TextRoundtrip)
{
    std::string msg="hello";
    auto out=webserver::build_text(msg);
    webserver::Frame f;
    size_t used=0;
    bool ok=webserver::parse_frame(out.data(), out.size(), f, used);
    ASSERT_TRUE(ok);
    ASSERT_EQ(f.opcode, webserver::OP_TEXT);
    ASSERT_EQ(std::string(f.payload.begin(), f.payload.end()), msg);
}
TEST(FrameTest, BinaryRoundtrip)
{
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    auto out=webserver::build_binary(data);
    webserver::Frame f;
    size_t used=0;
    bool ok=webserver::parse_frame(out.data(), out.size(), f, used);
    ASSERT_TRUE(ok);
    ASSERT_EQ(f.opcode, webserver::OP_BINARY);
    ASSERT_EQ(f.payload, data);
}
