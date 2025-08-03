//
// Created by Yue Xue  on 7/30/25.
//

#include "value.h"
#include "error.h"
#include "cclox_util.h"
#include <iostream>
#include <string>
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

Value operator==(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<A, B>) {
            // todo: modification is needed when reference is supported
            return a == b;
        } else {
            return false;
        }
    }, left, right);
}

Value operator>(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (are_non_bool_arithmetic<A, B>()) {
            // todo: modification is needed when reference is supported
            return a > b;
        } else {
            throw LoxTypeError("comparison is not supported for the given values");
        }
    }, left, right);
}

Value operator<(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (are_non_bool_arithmetic<A, B>()) {
            // todo: modification is needed when reference is supported
            return a < b;
        } else {
            throw LoxTypeError("comparison is not supported for the given values");
        }
    }, left, right);
}

Value operator+(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (are_non_bool_arithmetic<A, B>()) {
            return a + b;
        } else {
            throw LoxTypeError(fmt::format("the values do not support addition"));
        }

    }, left, right);
}

Value operator-(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (are_non_bool_arithmetic<A, B>()) {
            return a - b;
        } else {
            throw LoxTypeError(fmt::format("the values do not support subtraction"));
        }
    }, left, right);
}

Value operator*(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (are_non_bool_arithmetic<A, B>()) {
            return a * b;
        }  else {
            throw LoxTypeError(fmt::format("the values do not support multiplication"));
        }
    }, left, right);
}

Value operator/(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (are_non_bool_arithmetic<A, B>()) {
            return a / b;
        } else {
            throw LoxTypeError(fmt::format("the values do not support division"));
        }
    }, left, right);
}


