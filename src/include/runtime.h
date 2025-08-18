//
// Created by Yue Xue  on 8/4/25.
//

#ifndef CCLOX_RUNTIME_H
#define CCLOX_RUNTIME_H
#include <memory>
#include <vector>
#include "object.h"
#include "value.h"
#include "cclox_util.h"

class LoxObject;

namespace Runtime {

    extern std::vector<std::weak_ptr<LoxObject>> weak_pool;

    extern std::unordered_map<uint16_t, Value> globals;

    extern std::unordered_map<uint16_t, Value> builtin;

    extern std::mutex gc_lock;

    inline void register_object(const std::shared_ptr<LoxObject> &ptr) {
        weak_pool.push_back(std::weak_ptr{ptr});
    }

    extern std::atomic<size_t> allocated_size;

    // bool need_gc();

    /**
     * 估算本对象的内存占用（包括本对象的本体，但不包括其他loxobject本体），使gc的内存分配记录增加合适的值。
     * 该函数应该在本对象的内存占用被固定之后才使用，且仅能使用一次。如果有container成员，那么应该尽可能将其capacity先缩减为size，减少冗余
     */
    inline void record_allocation(const LoxReference &reference) {
        auto old = allocated_size.fetch_add(reference->compute_size());
        if (Flag::show_heap) {
            print_with_color(fmt::format("[+] heap: {:^6} -> {:^6}; {}\n", old, old + reference->compute_size(), reference->to_visual_string()), Color::BRIGHT_YELLOW);
        }
    }

    inline void record_free(LoxObject &lox_object) {
        auto old = allocated_size.fetch_sub(lox_object.compute_size());
        if (Flag::show_heap) {
            print_with_color(fmt::format("[-] heap: {:^6} -> {:^6}; {}\n", old, old - lox_object.compute_size(), lox_object.to_visual_string()), Color::BRIGHT_YELLOW);
        }
    }

    /**
     * 分配一个指定类型的对象，将其注册在weak_pool中。该函数的调用者必须手动在合适的时机进行record_allocation()
     * @return 对应类型的shared_ptr
     */
    template<typename T, typename... Args>
    static std::shared_ptr<T> allocate_as(Args... args) {
        static_assert(std::is_base_of_v<LoxObject, T>, "The template argument has to be a subclass of LoxObject");
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        register_object(ptr);
        return ptr;
    }

    /**
     * allocate_as的包装。返回一个LoxReference而非shared_ptr<T>，在某些情况下用起来更方便。该函数的调用者必须手动在合适的时机进行record_allocation()
     */
    template<typename T, typename... Args>
    static LoxReference allocate_as_ref(Args... args) {
        auto ptr = allocate_as<T>(std::forward<Args>(args)...);
        return std::static_pointer_cast<LoxObject>(ptr);
    }

}


#endif //CCLOX_RUNTIME_H
