//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_VALUE_H
#define CCLOX_VALUE_H
#include <any>
#include <thread>
#include <variant>
#include <memory>
#include "object.h"
#include "native.h"
#include "nativeobjects/loxconcurrent.h"

using NativeObject = std::variant<std::thread, LoxMutex, LoxCondition>;
using NativePair = std::pair<uint16_t, NativeObject>;
using NativeReference = std::shared_ptr<NativePair>;

using Value = std::variant<long, double, bool, std::nullptr_t, LoxReference, NativeReference, std::shared_ptr<LoxNativeFunction>>;

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
     * 将一个value静态地转化为一个LoxObject的子类的shared_ptr。
     * @pre 只有在确认该转化是正确的时候，才可以使用。
     */
    template<typename T>
    std::shared_ptr<T> to_reference_unsafe(const Value &value) {
        static_assert(std::is_base_of_v<LoxObject, T>);
        return std::static_pointer_cast<T>(std::get<LoxReference>(value));
    }

    /**
     * 如果给定的value是一个LoxReference，则对其进行mark。否则什么都不做。
     */
    void mark_value(Value &value, std::queue<LoxReference> &queue);
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
