//
// Created by Yue Xue  on 8/17/25.
//

#include "native.h"
#include "vm.h"

#include "runtime.h"
#include <chrono>
#include <thread>
#include <any>

#include "stringintern.h"

namespace Native {

    /**
     * 测试v是否是NativeReference，如果否，抛出异常。
     * 然后测试其是否是指定的T，如果否，抛出异常。
     * 如果没问题，则返回T的引用。
     * @throws LoxTypeError
     */
    template <typename T>
    T &test_native_object(Value &v, const std::string &expected) {
        if (std::holds_alternative<NativeReference>(v) == false) {
            throw LoxTypeError(fmt::format("invalid type of argument: {}, expect: <{}>", LoxValue::to_string(v), expected));
        }
        NativeReference ref = std::get<NativeReference>(v);
        if (std::holds_alternative<T>(ref->second) == false) {
            throw LoxTypeError(fmt::format("invalid type of argument: {}, expect: <{}>", LoxValue::to_string(v), expected));
        }
        return std::get<T>(ref->second);
    }


    /**
     * 获取当前的时间，单位为秒的浮点数
     * 无参数
     */
    void now(VM &vm, size_t fp) {
        vm.stack().pop_back();
        vm.stack().push_back(static_cast<double>(clock()) / CLOCKS_PER_SEC);
    }

    /**
     * 参数为一个closure或者class或者method
     */
    void thread_new(VM &vm, size_t fp) {
        Value v = vm.stack().back();
        vm.stack().pop_back(); // 弹出参数
        vm.stack().pop_back(); // 弹出该native
        uint16_t name_id = StringIntern::resolve_string("std::thread");
        if (std::holds_alternative<LoxReference>(v) == false) {
            throw LoxTypeError(fmt::format("{} is not callable and cannot be used to create a thread", LoxValue::to_string(v)));
        }
        LoxReference callable = std::get<LoxReference>(v);

        switch (callable->get_object_type()) {
            case LoxObjectType::Closure:{
                NativeReference t = std::make_shared<NativePair>(name_id, std::thread([callable]() {
                    VM new_vm;
                    new_vm.stack().push_back(callable);
                    new_vm.setup_frame(std::static_pointer_cast<LoxClosure>(callable), 0);
                    new_vm.run();
                }));
                vm.stack().push_back(t);
                break;
            }
            case LoxObjectType::Method: {
                break;
            }
            case LoxObjectType::Class: {
                break;
            }
            default: {
                throw LoxTypeError(fmt::format("{} is not callable and cannot be used to create a thread", callable->to_string()));
            }
        }
    }

    void thread_join(VM &vm, size_t fp) {
        Value v = vm.stack().back();
        vm.stack().pop_back(); // 弹出参数
        vm.stack().pop_back(); // 弹出该native
        auto &thrd = test_native_object<std::thread>(v, "std::thread");
        if (thrd.joinable()) {
            thrd.join();
        }
        vm.stack().push_back(nullptr); // 逻辑上返回nil。
    }

    void mutex_new(VM &vm, size_t fp) {
        vm.stack().pop_back();
        uint16_t name_id = StringIntern::resolve_string("pthread_mutex");
        auto v = std::make_shared<NativePair>(name_id, LoxMutex{});
        vm.stack().push_back(v);
    }

    void mutex_lock(VM &vm, size_t fp) {
        Value arg = vm.stack().back();
        vm.stack().pop_back();
        vm.stack().pop_back();
        auto &mutex = test_native_object<LoxMutex>(arg, "LoxMutex");
        mutex.lock();
        vm.stack().push_back(nullptr);
    }

    void mutex_unlock(VM &vm, size_t fp) {
        Value arg = vm.stack().back();
        vm.stack().pop_back();
        vm.stack().pop_back();
        auto &mutex = test_native_object<LoxMutex>(arg, "LoxMutex");
        mutex.unlock();
        vm.stack().push_back(nullptr);
    }

    void condition_new(VM &vm, size_t fp) {
        vm.stack().pop_back();
        uint16_t name_id = StringIntern::resolve_string("LoxCondition");
        auto v = std::make_shared<NativePair>(name_id, LoxCondition{});
        vm.stack().push_back(v);
    }

    void condition_wait(VM &vm, size_t fp) {
        // condition_wait(cv, mutex);
        Value arg2 = vm.stack().back();
        vm.stack().pop_back();
        Value arg1 = vm.stack().back();
        vm.stack().pop_back();
        auto &cv = test_native_object<LoxCondition>(arg1, "LoxCondition");
        auto &mutex = test_native_object<LoxMutex>(arg2, "LoxMutex");
        cv.wait(mutex);
        vm.stack().push_back(nullptr);
    }

    void condition_notify_one(VM &vm, size_t fp) {
       // condition_notify_one(cv);

        Value v = vm.stack().back();
        vm.stack().pop_back();
        auto &cv = test_native_object<LoxCondition>(v, "LoxCondition");
        cv.notify();
        vm.stack().push_back(nullptr);
    }
}

void add_native(const std::string &name, LoxNativeFunction::NativeImpl impl, int arity) {
    auto native = std::make_shared<LoxNativeFunction>(impl, name, arity);
    auto str_id = StringIntern::resolve_string(name);
    Runtime::builtin.insert({str_id, native});
}

void load_all_natives() {
    add_native("now", Native::now, 0);
    add_native("thread_new", Native::thread_new, 1);
    add_native("thread_join", Native::thread_join, 1);
    add_native("mutex_new", Native::mutex_new, 0);
    add_native("mutex_lock", Native::mutex_lock, 1);
    add_native("mutex_unlock", Native::mutex_unlock, 1);
    add_native("condition_new", Native::condition_new, 0);
    add_native("condition_wait", Native::condition_wait, 2);
    add_native("condition_notify_one", Native::condition_notify_one, 1);
}




