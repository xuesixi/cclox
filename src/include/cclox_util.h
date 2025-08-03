//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_CCLOX_UTIL_H
#define CCLOX_CCLOX_UTIL_H

#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
#include <limits>
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

template <typename T>
bool within(size_t num) {
    return num <= std::numeric_limits<T>::max();
}

template <typename T>
constexpr bool is_non_bool_arithmetic() {
    return !std::is_same_v<T, bool> && std::is_arithmetic_v<T>;
}

template <typename T, typename U>
constexpr bool are_non_bool_arithmetic() {
    return is_non_bool_arithmetic<T>() && is_non_bool_arithmetic<U>();
}

// 打开指定路径的文件，并将其全部内容返回为一个string。如果文件无法被打开，抛出runtime_err
std::string read_file(const std::string &path);

#endif //CCLOX_CCLOX_UTIL_H
