//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_CCLOX_UTIL_H
#define CCLOX_CCLOX_UTIL_H

#include "common.h"
#include <fstream>
#include <limits>
#include <sstream>
#include <tuple>

enum class Color {
    None,
    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
    BRIGHT_BLACK, BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW,
    BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
};

/**
 * @return 一个yyyymmdd_HHMMSS格式的时间戳字符串
 */
std::string timestamp_str();

std::string nanos_str();

extern const std::chrono::time_point<std::chrono::steady_clock> program_start;

/**
 * 带颜色的输出
 * @param out 输出流。一个文件流或者cout
 * @param content 内容
 * @param color 颜色。Color::None为无色。如果Flag::print_color为false，那么无论传入什么颜色，都不会打印颜色
 */
void print_to(std::ostream &out ,const std::string &content, Color color);

//
/**
 * 打开指定路径的文件，并将其全部内容返回为一个string。
 * @param path 路径
 * @return 文件内容
 * @throws FileOpenFailureError 文件打开异常
 */
std::string read_file(const std::string &path);


/**
 * 返回值的第一个是high，第二个是low
 */
inline std::pair<uint8_t, uint8_t> u16_to_u8(uint16_t num) {
    return {
        (num >> 8) & 0x00FF,
        num & 0x00FF
    };
}

/**
 * 将两个uint8转化为一个uint16
 * @param low 低位
 * @param high 高位
 */
inline uint16_t u8_to_u16(uint8_t low, uint8_t high) {
    return (high << 8) | low;
}

/**
 * 以1e-8为门槛判断两个double是否相同
 */
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

template <typename A>
size_t estimate_string_map_size(std::unordered_map<std::string, A> map) {

    size_t total_bytes = 0;

    // 2. Bucket array memory
    // Each bucket typically stores a pointer to the first node in that bucket
    total_bytes += map.bucket_count() * sizeof(void*);

    // 3. Node memory
    // Each element is typically stored in a node with: key, value, next pointer, hash
    const size_t node_overhead = sizeof(void*) + sizeof(size_t); // next pointer + hash
    const size_t per_node_size = sizeof(std::string) + sizeof(A) + node_overhead;
    for (const auto &pair: map) {
        total_bytes += per_node_size;
        total_bytes += pair.first.capacity();
    }
    return total_bytes;
}

template <typename A>
size_t estimate_u16_map_size(std::unordered_map<uint16_t, A> map) {

    size_t total_bytes = 0;

    // 2. Bucket array memory
    // Each bucket typically stores a pointer to the first node in that bucket
    total_bytes += map.bucket_count() * sizeof(void*);

    // 3. Node memory
    // Each element is typically stored in a node with: key, value, next pointer, hash
    const size_t node_overhead = sizeof(void*) + sizeof(size_t); // next pointer + hash
    const size_t per_node_size = sizeof(uint16_t) + sizeof(A) + node_overhead;
    total_bytes += map.size() * per_node_size;
    // for (const auto &pair: map) {
    //     total_bytes += per_node_size;
    //     total_bytes += pair.first.capacity();
    // }
    return total_bytes;
}



#endif //CCLOX_CCLOX_UTIL_H
