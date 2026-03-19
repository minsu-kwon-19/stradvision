#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace core {
namespace message {

/**
 * @brief Base interface for all message payloads.
 * Ensures that all payloads provide a consistent serialization method.
 */
class IPayload {
   public:
    virtual ~IPayload() = default;

    /**
     * @brief Serializes the payload into a byte vector.
     * @return std::vector<uint8_t> The serialized payload data.
     */
    virtual std::vector<uint8_t> serialize() const = 0;

    /**
     * @brief Deserializes data into the current payload object.
     * @param data The byte vector containing payload data.
     */
    virtual void deserialize(const std::vector<uint8_t>& data) = 0;
};

}  // namespace message
}  // namespace core
