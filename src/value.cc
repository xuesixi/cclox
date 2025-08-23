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

#include "runtime.h"
#include "stringintern.h"

std::string LoxValue::to_string(const Value &value) {
    return std::visit([](auto &&arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, LoxReference>) {
            return arg->to_string();
        } else if constexpr (std::is_same_v<T, nullptr_t>) {
            return "nil";
        } else if constexpr (std::is_same_v<T, NativeReference>) {
            return fmt::format("<cc: {}>", StringIntern::read_from_id(arg->first));
        } else if constexpr (std::is_same_v<T, std::shared_ptr<LoxNativeFunction>>) {
            return arg->get_name();
        } else {
            return fmt::format("{}", arg);
        }
    }, value);
}

std::string LoxValue::to_visual_string(const Value &value) {
    return std::visit([](auto &&arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, LoxReference>) {
            return arg->to_visual_string();
        } else if constexpr (std::is_same_v<T, nullptr_t>) {
            return "nil";
        } else if constexpr (std::is_same_v<T, NativeReference>) {
            return fmt::format("<cc: {}>", StringIntern::read_from_id(arg->first));
        } else if constexpr (std::is_same_v<T, std::shared_ptr<LoxNativeFunction>>) {
            return arg->get_name();
        } else {
            return fmt::format("{}", arg);
        }
    }, value);
}

void LoxValue::mark_value(Value &value, std::queue<LoxReference> &queue) {
    if (std::holds_alternative<LoxReference>(value)) {
        LoxObject::mark(std::get<LoxReference>(value), queue);
    }
}

Value operator-(const Value &value) {
    if (std::holds_alternative<long>(value)) {
        return -std::get<long>(value);
    } else if (std::holds_alternative<double>(value)) {
        return -std::get<double>(value);
    } else {
        throw LoxTypeError(fmt::format("the values {} does not support negation", LoxValue::to_string(value)));
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

Value LoxValue::power(const Value &left, const Value &right) {
    return std::visit([](auto &&a, auto &&b) -> Value {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;
        if constexpr (are_non_bool_arithmetic<A, B>()) {
            return std::pow(a, b);
        } else {
            throw LoxTypeError(fmt::format("the values {} and {} do not support power arithmetic", LoxValue::to_string(a), LoxValue::to_string(b)));
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
            throw LoxTypeError(fmt::format("the values {} and {} do not support comparison", LoxValue::to_string(a), LoxValue::to_string(b)));
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
            throw LoxTypeError(fmt::format("the values {} and {} do not support comparison", LoxValue::to_string(a), LoxValue::to_string(b)));
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
            if (a->is_of_type(LoxObjectType::String) && b->is_of_type(LoxObjectType::String)) {
                auto a_str = std::static_pointer_cast<LoxString>(a);
                auto b_str = std::static_pointer_cast<LoxString>(b);

                LoxReference str = Runtime::allocate_as_ref<LoxString>(*a_str + *b_str);
                Runtime::record_allocation(str);
                return str;
            } else {
                throw LoxTypeError(fmt::format("the values {} and {} do not support addition", LoxValue::to_string(a), LoxValue::to_string(b)));
            }
        } else {
            throw LoxTypeError(fmt::format("the values {} and {} do not support addition", LoxValue::to_string(a), LoxValue::to_string(b)));
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
            throw LoxTypeError(fmt::format("the values {} and {} do not support subtraction", LoxValue::to_string(a), LoxValue::to_string(b)));
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
            throw LoxTypeError(fmt::format("the values {} and {} do not support multiplication", LoxValue::to_string(a), LoxValue::to_string(b)));
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
            throw LoxTypeError(fmt::format("the values {} and {} do not support division", LoxValue::to_string(a), LoxValue::to_string(b)));
        }
    }, left, right);
}


