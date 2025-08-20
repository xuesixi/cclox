//
// Created by Yue Xue  on 8/4/25.
//

#include "runtime.h"

namespace Runtime {

    std::vector<std::weak_ptr<LoxObject>> weak_pool;

    std::atomic<size_t> allocated_size = 0;

    std::unordered_map<uint16_t, Value> globals;

    std::unordered_map<uint16_t, Value> builtin;

    std::mutex gc_lock;

    std::mutex globals_access_mutex;

    bool need_gc() {

    }

    void check_gc() {
        std::lock_guard<std::mutex> lock(gc_lock);
        if (need_gc() == false) {
            return;
        }
        std::thread{gc};
    }

    void gc() {
        std::lock_guard<std::mutex> lock(gc_lock);
        std::vector<std::weak_ptr<LoxObject>> new_pool;
        new_pool.reserve(weak_pool.size() / 2);
        std::ranges::copy_if(weak_pool, std::back_inserter(new_pool), [](const std::weak_ptr<LoxObject> &weak) {
            return !weak.expired();
        });
        weak_pool = new_pool;

    }

}
