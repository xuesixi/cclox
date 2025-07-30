//
// Created by Yue Xue  on 7/30/25.
//

#include "value.h"
#include <string>
#include <fmt/core.h>

std::string value_to_string(Value value) {
    return std::visit([](auto &&arg) {
        return fmt::format("{}", arg);
//        using T = std::decay_t<decltype(arg)>;
//        if constexpr (std::is_same_v<long, T>) {
//            return fmt::format("{}", arg);
//        } else if constexpr (std::is_same_v<bool, T> ) {
//            return fmt::format("{}", arg);
//        } else if constexpr (std::is_same_v<double, T>) {
//
//        }
    }, value);
}
