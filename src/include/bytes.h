//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_BYTES_H
#define CCLOX_BYTES_H

#include <cstdint>
#include <tuple>

inline std::pair<uint8_t, uint8_t> u16_to_u8(uint16_t num) {
    return {
        (num >> 8) & 0x00FF,
        num & 0x00FF
    };
}

inline uint16_t u8_to_u16(uint8_t low, uint8_t high) {
    return (high << 8) | low;
}

#endif //CCLOX_BYTES_H
