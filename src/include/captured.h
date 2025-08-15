//
// Created by Yue Xue  on 8/12/25.
//

#ifndef CAPTURED_H
#define CAPTURED_H

#include "value.h"

/**
 * 被捕获值。有两种状态：在栈上，或者已逃逸
 */
class Captured {
public:
    Captured() = default;

    Captured(std::vector<Value> &stack, size_t index): data(OpenRef{&stack, index}) {
    }

    /**
     * @return 该值在栈上的索引
     * @pre 尚未逃逸（仍在栈上）
     */
    size_t index() const {
        return std::get<OpenRef>(data).second;
    }

    /**
     * 逃逸，也就是将捕获的值从栈上转移到该对象内部。只能调用一次。
     */
    void escape() {
        auto [stack, index] = std::get<OpenRef>(data);
        data = stack->at(index);
    }

    /**
     * @return 该捕获值的Value。会根据是否逃逸返回合适的结果。调用者无需关心是否已经逃逸
     */
    Value &value() {
        if (std::holds_alternative<OpenRef>(data)) {
            auto [stack, index] = std::get<OpenRef>(data);
            return stack->at(index);
        } else {
            return std::get<Value>(data);
        }
    }

private:
    using OpenRef = std::pair<std::vector<Value> *, size_t>; // <stack, index>
    std::variant<OpenRef, Value> data;
};


#endif //CAPTURED_H
