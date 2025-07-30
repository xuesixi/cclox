//
// Created by Yue Xue  on 7/30/25.
//

#include "value.h"
#include <iostream>
#include <string>
#include "error.h"
#include <fmt/core.h>
#include <type_traits>
#include <variant>

std::string LoxValue::to_string(const Value &value) {
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

void LoxValue::print(const Value &value) {
    std::cout << LoxValue::to_string(value) << std::endl;
}

Value operator-(const Value &value) {
    if (std::holds_alternative<long>(value)) {
        return - std::get<long>(value);
    } else if (std::holds_alternative<double>(value)) {
        return - std::get<double>(value);
    } else {
        throw LoxTypeError(fmt::format("the value {} does not support negation", LoxValue::to_string(value)));
    }
}

Value operator+(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<A, bool> || std::is_same_v<B, bool>) {
            throw LoxTypeError(fmt::format("bool does not support addition"));
        } else if constexpr (std::is_same_v<A, B>) {
            return Value {a + b};
        } else if constexpr (std::is_arithmetic_v<A> && std::is_arithmetic_v<B>) {
            return Value {static_cast<double>(a) + static_cast<double>(b)};
        } else {
            throw LoxTypeError(fmt::format("the values [{} and {}] do not support addition", a, b));
        }

    }, left, right);
}

Value operator-(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<A, bool> || std::is_same_v<B, bool>) {
            throw LoxTypeError(fmt::format("bool does not support substraction"));
        } else if constexpr (std::is_same_v<A, B>) {
            return Value {a - b};
        } else if constexpr (std::is_arithmetic_v<A> && std::is_arithmetic_v<B>) {
            return Value {static_cast<double>(a) - static_cast<double>(b)};
        } else {
            throw LoxTypeError(fmt::format("the values do not support substraction"));
        }
    }, left, right);
}

Value operator*(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<A, bool> || std::is_same_v<B, bool>) {
            throw LoxTypeError(fmt::format("bool does not support multiplication"));
        } else if constexpr (std::is_same_v<A, B>) {
            return Value {a * b};
        } else if constexpr (std::is_arithmetic_v<A> && std::is_arithmetic_v<B>) {
            return Value {static_cast<double>(a) * static_cast<double>(b)};
        } else {
            throw LoxTypeError(fmt::format("the values do not support multiplication"));
        }
    }, left, right);
}

Value operator/(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<A, bool> || std::is_same_v<B, bool>) {
            throw LoxTypeError(fmt::format("bool does not support division"));
        } else if constexpr (std::is_same_v<A, B>) {
            return Value {a / b};
        } else if constexpr (std::is_arithmetic_v<A> && std::is_arithmetic_v<B>) {
            return Value {static_cast<double>(a) / static_cast<double>(b)};
        } else {
            throw LoxTypeError(fmt::format("the values do not support division"));
        }
    }, left, right);
}


