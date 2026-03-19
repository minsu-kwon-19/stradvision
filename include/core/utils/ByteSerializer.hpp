#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

namespace core {
namespace utils {

inline void write_u16_be(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back(val & 0xFF);
}

inline uint16_t read_u16_be(const uint8_t* buf) {
    return (uint16_t(buf[0]) << 8) | uint16_t(buf[1]);
}

inline void write_u32_be(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back((val >> 24) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back(val & 0xFF);
}

inline uint32_t read_u32_be(const uint8_t* buf) {
    return (uint32_t(buf[0]) << 24) | (uint32_t(buf[1]) << 16) | (uint32_t(buf[2]) << 8) |
           uint32_t(buf[3]);
}

inline void write_u64_be(std::vector<uint8_t>& buf, uint64_t val) {
    write_u32_be(buf, val >> 32);
    write_u32_be(buf, val & 0xFFFFFFFF);
}

inline uint64_t read_u64_be(const uint8_t* buf) {
    return (uint64_t(read_u32_be(buf)) << 32) | read_u32_be(buf + 4);
}

inline void write_float_be(std::vector<uint8_t>& buf, float val) {
    uint32_t ival;
    std::memcpy(&ival, &val, sizeof(float));
    write_u32_be(buf, ival);
}

inline float read_float_be(const uint8_t* buf) {
    uint32_t ival = read_u32_be(buf);
    float    val;
    std::memcpy(&val, &ival, sizeof(float));
    return val;
}

}  // namespace utils
}  // namespace core
