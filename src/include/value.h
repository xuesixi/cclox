//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_VALUE_H
#define CCLOX_VALUE_H
#include <variant>
#include <memory>

using Value = std::variant<long, double, bool>;

namespace LoxValue {
    std::string to_string(const Value &value);
    void print(const Value &value);
}

Value operator-(const Value &value);
Value operator-(const Value &a, const Value &b);
Value operator+(const Value &a, const Value &b);
Value operator*(const Value &a, const Value &b);
Value operator/(const Value &a, const Value &b);

#endif //CCLOX_VALUE_H
