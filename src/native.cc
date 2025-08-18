//
// Created by Yue Xue  on 8/17/25.
//

#include "objects/loxnative.h"
#include "native.h"
#include "runtime.h"
#include <chrono>

namespace Native {
    /**
     * 获取当前的时间，单位为秒的浮点数
     */
    void now(Stack &stack, size_t fp) {
        stack.pop_back();
        stack.push_back(static_cast<double>(clock()) / CLOCKS_PER_SEC);
    }
}

void add_native(const std::string &name, LoxNative::NativeImpl impl, int arity) {
    auto native = Runtime::allocate_as_ref<LoxNative>(impl, name, arity);
    Runtime::record_allocation(native);
    Runtime::builtin.insert({name, native});
}

void load_all_natives() {
    add_native("now", Native::now, 0);
}




