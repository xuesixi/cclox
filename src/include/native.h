//
// Created by Yue Xue  on 8/17/25.
//

#ifndef NATIVE_H
#define NATIVE_H

#include "objects/loxnative.h"
#include "vm.h"

/**
 * 这个命名空间中是一系列NativeImpl，也就是LoxNative的核心。
 * 在编写新的NativeImpl时，请注意以下几点：
 * - 调用时，栈的状态是[impl, arg1, arg2, args...]。NativeImpl必须自行弹出自身以及所有参数
 * - 参数fp代表的是在调用时，impl处于栈中的位置。
 * - NativeImpl在形式上的返回值为void。如果该函数在逻辑上有返回值，那么将其置入栈顶。如果在逻辑上没有返回值，那么必须将Nil置入栈顶。
 */
namespace Native {
    void now(VM &vm, size_t fp);
    void thread_new(VM &vm, size_t fp);
    void thread_join(VM &vm, size_t fp);
    void mutex_new(VM &vm, size_t fp);
    void mutex_lock(VM &vm, size_t fp);
    void mutex_unlock(VM &vm, size_t fp);
    void condition_new(VM &vm, size_t fp);
    void condition_wait(VM &vm, size_t fp);
    void condition_notify_one(VM &vm, size_t fp);
    void condition_notify_all(VM &vm, size_t fp);
}

/**
 * 将所有的LoxNative对象置入Runtime::builtin。
 * 请在main中调用该函数一次。
 */
void load_all_natives();

#endif //NATIVE_H
