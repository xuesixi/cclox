//
// Created by Yue Xue  on 8/12/25.
//

#ifndef CAPTURED_H
#define CAPTURED_H

#include "value.h"

class Captured {
public:
    Captured() = default;

    Captured(std::vector<Value> &stack, size_t index): data(OpenRef{&stack, index}) {
    }

    /**
     * @return 该值在栈上的索引
     */
    size_t index() {
        return std::get<OpenRef>(data).second;
    }

    /**
     * 将捕获的值从栈上转移到该对象内部。只能调用一次。
     */
    void escape() {
        auto [stack, index] = std::get<OpenRef>(data);
        data = stack->at(index);
    }

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
