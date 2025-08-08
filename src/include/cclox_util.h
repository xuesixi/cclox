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

/**
 * 返回值的第一个是high，第二个是low
 */
inline std::pair<uint8_t, uint8_t> u16_to_u8(uint16_t num) {
    return {
        (num >> 8) & 0x00FF,
        num & 0x00FF
    };
}

inline uint16_t u8_to_u16(uint8_t low, uint8_t high) {
    return (high << 8) | low;
}

inline bool double_equal(double a, double b) {
    return abs(a - b) < 1e-8;
}

// num 是否处于T类型的数值极限之内
template<typename T>
bool within(size_t num) {
    return num <= std::numeric_limits<T>::max();
}

// T 是否是非bool的可计算类型
template<typename T>
constexpr bool is_non_bool_arithmetic() {
    return !std::is_same_v<T, bool> && std::is_arithmetic_v<T>;
}

// T和U是否都是非bool的可计算类型
template<typename T, typename U>
constexpr bool are_non_bool_arithmetic() {
    return is_non_bool_arithmetic<T>() && is_non_bool_arithmetic<U>();
}

// T, U, V三者是否相同
template<typename T, typename U, typename V>
constexpr bool are_same() {
    return std::is_same_v<T, V> && std::is_same_v<U, V>;
}

// 打开指定路径的文件，并将其全部内容返回为一个string。如果文件无法被打开，抛出runtime_err
std::string read_file(const std::string &path);


#endif //CCLOX_CCLOX_UTIL_H
