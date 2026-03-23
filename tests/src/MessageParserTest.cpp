#include <gtest/gtest.h>

#include <vector>

#include "core/message/BinaryMessageParser.hpp"
#include "core/message/Payloads.hpp"

using namespace core::message;

class BinaryMessageParserTest : public ::testing::Test {
   protected:
    BinaryMessageParser parser;
};

// Test valid serialization and deserialization
TEST_F(BinaryMessageParserTest, CrcPassesOnValidData) {
    auto         msg = std::make_shared<Message>(MessageType::HELLO);
    HelloPayload payload{100, 1};
    msg->payload            = payload.serialize();
    msg->header.agent_id    = 100;
    msg->header.header_id   = 1234;
    msg->header.payload_len = static_cast<uint32_t>(msg->payload.size());

    // appends the CRC
    auto encoded = parser.serialize(*msg);

    size_t consumed = 0;

    auto decoded_msg = parser.deserialize(encoded, consumed);

    EXPECT_NE(decoded_msg, nullptr);
    EXPECT_EQ(consumed, encoded.size());
    EXPECT_EQ(decoded_msg->header.agent_id, 100);
}

// Test detection of modified CRC checksum
TEST_F(BinaryMessageParserTest, ThrowsOnTamperedCrc) {
    auto         msg = std::make_shared<Message>(MessageType::HELLO);
    HelloPayload payload{100, 1};
    msg->payload            = payload.serialize();
    msg->header.payload_len = static_cast<uint32_t>(msg->payload.size());

    auto encoded = parser.serialize(*msg);

    encoded.back() ^= 0xFF;

    size_t consumed = 0;
    EXPECT_THROW({ parser.deserialize(encoded, consumed); }, std::runtime_error);
}

// Test detection of corrupted payload data
TEST_F(BinaryMessageParserTest, ThrowsOnTamperedData) {
    auto         msg = std::make_shared<Message>(MessageType::HELLO);
    HelloPayload payload{100, 1};
    msg->payload            = payload.serialize();
    msg->header.payload_len = static_cast<uint32_t>(msg->payload.size());

    auto encoded = parser.serialize(*msg);

    if (encoded.size() > 14) {
        encoded[13] ^= 0xFF;
    }

    size_t consumed = 0;
    EXPECT_THROW({ parser.deserialize(encoded, consumed); }, std::runtime_error);
}
