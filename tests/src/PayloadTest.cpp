#include <gtest/gtest.h>
#include "core/message/Payloads.hpp"

using namespace core::message;

TEST(PayloadTest, HelloPayload_Parsing) {
    HelloPayload original{123, 1};
    auto buf = original.serialize();
    HelloPayload decoded;
    decoded.deserialize(buf);
    EXPECT_EQ(decoded.agent_id, 123);
    EXPECT_EQ(decoded.version, 1);
}

TEST(PayloadTest, HeartbeatPayload_Parsing) {
    HeartbeatPayload original;
    auto buf = original.serialize();
    HeartbeatPayload decoded;
    decoded.deserialize(buf);
    // HeartbeatPayload currently carries no internal fields, only Header data
    EXPECT_TRUE(true); 
}

TEST(PayloadTest, StatePayload_Parsing) {
    StatePayload original{2, 85.5f, 40.0f, 1};
    auto buf = original.serialize();
    StatePayload decoded;
    decoded.deserialize(buf);
    EXPECT_EQ(decoded.mode, 2);
    EXPECT_FLOAT_EQ(decoded.load, 85.5f);
    EXPECT_FLOAT_EQ(decoded.temp, 40.0f);
    EXPECT_EQ(decoded.last_rc, 1);
}

TEST(PayloadTest, AckPayload_Parsing) {
    AckPayload original{999};
    auto buf = original.serialize();
    AckPayload decoded;
    decoded.deserialize(buf);
    EXPECT_EQ(decoded.cmd_id, 999);
}

TEST(PayloadTest, NackPayload_Parsing) {
    NackPayload original{888, "Invalid Command"};
    auto buf = original.serialize();
    NackPayload decoded;
    decoded.deserialize(buf);
    EXPECT_EQ(decoded.cmd_id, 888);
    EXPECT_EQ(decoded.reason, "Invalid Command");
}

TEST(PayloadTest, CmdSetModePayload_Parsing) {
    CmdSetModePayload original{3};
    auto buf = original.serialize();
    CmdSetModePayload decoded;
    decoded.deserialize(buf);
    EXPECT_EQ(decoded.mode, 3);
}
