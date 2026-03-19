#include <gtest/gtest.h>
#include "core/message/BinaryMessageParser.hpp"
#include "core/message/Payloads.hpp"

using namespace core::message;

TEST(ProtocolTest, EncodeDecodeHello) {
    HelloPayload hp{123, 1};
    Message      msg(MessageType::HELLO, hp.serialize(), 123, 1);

    BinaryMessageParser proto;
    auto                encoded = proto.serialize(msg);
    EXPECT_GT(encoded.size(), Header::size());

    size_t consumed = 0;
    auto   decoded  = proto.deserialize(encoded, consumed);

    ASSERT_NE(decoded, nullptr);
    EXPECT_EQ(decoded->header.type, MessageType::HELLO);
    EXPECT_EQ(decoded->header.agent_id, 123);
    EXPECT_EQ(decoded->header.header_id, 1);
    EXPECT_EQ(consumed, encoded.size());

    HelloPayload decoded_hp;
    decoded_hp.deserialize(decoded->payload);
    EXPECT_EQ(decoded_hp.agent_id, 123);
    EXPECT_EQ(decoded_hp.version, 1);
}

TEST(ProtocolTest, CrcMismatch) {
    HelloPayload hp{123, 1};
    Message      msg(MessageType::HELLO, hp.serialize(), 123, 1);

    BinaryMessageParser proto;
    auto                encoded = proto.serialize(msg);

    // Corrupt the CRC (last 4 bytes)
    encoded.back() ^= 0xFF;

    size_t consumed = 0;
    EXPECT_THROW(proto.deserialize(encoded, consumed), std::runtime_error);
}
