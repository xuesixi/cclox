//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_CCLOX_UTIL_H
#define CCLOX_CCLOX_UTIL_H

#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
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

std::string read_file(const std::string &path) {
    std::ifstream ifs(path);
    return std::string(std::istreambuf_iterator<char>(ifs),std::istreambuf_iterator<char>());
}


#endif //CCLOX_CCLOX_UTIL_H
