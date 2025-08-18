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

    // void gc() {
    //     std::lock_guard<std::mutex> lock(gc_lock);
    //     if (need_gc() == false) {
    //         return;
    //     }
    //     std::vector<std::weak_ptr<LoxObject>> new_pool;
    //     for (auto & weak : weak_pool) {
    //         if (weak.expired()) {
    //             continue;
    //         }
    //         new_pool.emplace_back(weak);
    //     }
    //     weak_pool = new_pool;
    //
    // }

}
