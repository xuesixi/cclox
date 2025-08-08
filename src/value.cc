//
// Created by Yue Xue  on 7/30/25.
//

#include "value.h"
#include "objects/loxstring.h"
#include "error.h"
#include "cclox_util.h"
#include <iostream>
#include <string>
#include <fmt/core.h>
#include <type_traits>
#include <variant>

std::string LoxValue::to_string(const Value &value) {
    return std::visit([](auto &&arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, LoxReference>) {
            return arg->to_string();
        } else if constexpr (std::is_same_v<T, nullptr_t>) {
            return "nil";
        } else {
            return fmt::format("{}", arg);
        }
    }, value);
}

void LoxValue::print(const Value &value) {
    std::cout << LoxValue::to_string(value) << std::endl;
}

Value operator-(const Value &value) {
    if (std::holds_alternative<long>(value)) {
        return -std::get<long>(value);
    } else if (std::holds_alternative<double>(value)) {
        return -std::get<double>(value);
    } else {
        throw LoxTypeError(fmt::format("the value {} does not support negation", LoxValue::to_string(value)));
    }
}

Value operator==(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<A, B>) {
            if constexpr (std::is_same_v<A, LoxReference>) {
                return *a == *b;
            } else {
                return a == b;
            }
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
        } else if constexpr (are_same<A, B, LoxReference>()) {
            // LoxString是唯一支持+的引用类型
            auto *a_str = LoxValue::to_reference<LoxString>(a);
            auto *b_str = LoxValue::to_reference<LoxString>(b);
            if (a_str && b_str) {
                return LoxObject::allocate<LoxString>(*a_str + *b_str);
            } else {
                throw LoxTypeError(fmt::format("the values do not support addition"));
            }
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
        } else {
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


