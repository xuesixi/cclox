//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_VALUE_H
#define CCLOX_VALUE_H
#include <variant>
#include <memory>
#include "object.h"

using Value = std::variant<long, double, bool, nullptr_t, LoxReference>;

namespace LoxValue {
    std::string to_string(const Value &value);
    void print(const Value &value);
    inline bool to_bool(const Value &value) {
        return !(std::holds_alternative<nullptr_t>(value) || (std::holds_alternative<bool>(value) && !std::get<bool>(value)));
    }

    /**
     * 将一个LoxReference动态转化为T类型的指针，如果失败，返回nullptr。
     */
    template <typename T>
    T *to_reference(const LoxReference& ref) {
        return dynamic_cast<T*>(ref.get());
    }

    /**
     * 将一个value动态地转化为T类型的指针。使用者必须在使用前自行确认该Value真的是一个LoxReference
     */
    template <typename T>
    T *to_reference(const Value &value) {
        return to_reference<T>(std::get<LoxReference >(value));
    }
}

Value operator-(const Value &value);
Value operator-(const Value &a, const Value &b);
Value operator+(const Value &a, const Value &b);
Value operator*(const Value &a, const Value &b);
Value operator/(const Value &a, const Value &b);
Value operator>(const Value &a, const Value &b);
Value operator<(const Value &a, const Value &b);
Value operator==(const Value &a, const Value &b);

#endif //CCLOX_VALUE_H
