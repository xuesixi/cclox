//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_VALUE_H
#define CCLOX_VALUE_H
#include <variant>
#include <memory>
#include "object.h"

using Value = std::variant<long, double, bool, std::nullptr_t, LoxReference>;

namespace LoxValue {

    /**
     * 一个Value的字符串表达
     */
    std::string to_string(const Value &value);

    std::string to_visual_string(const Value &value);

    /**
     * 幂运算
     */
    Value power(const Value &left, const Value &right);

    /**
     * 将一个值转化为对应的布尔值。
     * @return 如果是nil或者false，则为false。除此之外的所有值都是true
     */
    inline bool to_bool(const Value &value) {
        return !(std::holds_alternative<std::nullptr_t>(value) || (
                     std::holds_alternative<bool>(value) && !std::get<bool>(value)));
    }

    /**
     * 试图将一个LoxReference动态转化为T类型的指针，如果失败，返回nullptr。
     */
    template<typename T>
    T *try_cast(const LoxReference &ref) {
        return dynamic_cast<T *>(ref.get());
    }

    /**
     * 将一个value动态地转化为T类型的指针
     * @pre Value 是一个LoxReference
     */
    template<typename T>
    T *try_cast(const Value &value) {
        return try_cast<T>(std::get<LoxReference>(value));
    }

    /**
     * 将一个value静态地转化为一个LoxObject的子类的shared_ptr。
     * @pre 只有在确认该转化是正确的时候，才可以使用。
     */
    template<typename T>
    std::shared_ptr<T> to_reference_unsafe(const Value &value) {
        static_assert(std::is_base_of_v<LoxObject, T>);
        return std::static_pointer_cast<T>(std::get<LoxReference>(value));
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
