//
// Created by Yue Xue  on 8/17/25.
//

#ifndef NATIVE_H
#define NATIVE_H

#include "objects/loxnative.h"

/**
 * 这个命名空间中是一系列NativeImpl，也就是LoxNative的核心。
 */
namespace Native {
    using Stack = std::vector<Value>;

    void now(Stack &stack, size_t fp);
}

/**
 * 将所有的LoxNative对象置入Runtime::builtin。
 * 请在main中调用该函数一次。
 */
void load_all_natives();

#endif //NATIVE_H
