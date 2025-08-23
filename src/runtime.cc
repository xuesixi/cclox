//
// Created by Yue Xue  on 8/4/25.
//

#include "runtime.h"

#include <latch>

#include "cclox_util.h"
#include "common.h"
#include "fmt/core.h"
#include "vm.h"

namespace Runtime {

    std::vector<std::weak_ptr<LoxObject>> weak_pool;

    std::atomic<size_t> allocated_size = 0;

    std::unordered_map<uint16_t, Value> globals;

    std::unordered_map<uint16_t, Value> builtin;

    std::mutex gc_mutex;

    std::atomic<bool> in_gc = false;

    std::optional<std::latch> pause_latch;

    std::optional<std::latch> resume_latch;

    std::mutex globals_access_mutex;

    bool allow_gc = false;

    /**
     * 当一个新的vm被创建的时候，会被添加入该列表。当销毁时，从中移除
     */
    std::list<VM *> vm_list;

    bool need_gc() {
        //todo
        return allow_gc;
    }

    void check_gc() {
        std::lock_guard<std::mutex> lock{gc_mutex};
        if (need_gc() == false) {
            return;
        }
        in_gc = true;
        if (Flag::show_heap) {
            print_with_color(fmt::format("gc starts with allocated size : {}\n", allocated_size.load()), Color::RED);
        }

        pause_latch.emplace(vm_list.size());
        resume_latch.emplace(vm_list.size());

        // 在此等待，直到所有线程都响应
        pause_latch->arrive_and_wait();

        // 接下来这段区间内，只有gc的vm线程在运行，其他线程都在等待

        // 标记根节点
        std::queue<LoxReference> queue = mark_roots();

        // 追踪标记
        while (!queue.empty()) {
            auto ref = queue.front();
            queue.pop();
            ref->mark_reference(queue);
        }

        // 清除
        for (auto & weak : weak_pool) {
            auto ptr = weak.lock();
            if (ptr and !ptr->is_marked and !ptr->is_protected) {
                ptr->clear_reference();
            }
        }

        // 更新weak_pool
        std::vector<std::weak_ptr<LoxObject>> new_pool;

        for (auto & weak : weak_pool) {
            auto ptr = weak.lock();
            if (ptr) {
                ptr->is_marked = false;
                new_pool.push_back(weak);
            }
        }

        weak_pool = new_pool;

        in_gc = false;

        // 唤醒所有线程
        resume_latch->arrive_and_wait();

        if (Flag::show_heap) {
            print_with_color(fmt::format("gc ends with allocated size : {}\n", allocated_size.load()), Color::RED);
        }

    }

    std::queue<LoxReference> mark_roots() {
        std::queue<LoxReference> queue;
        for (auto & pair : globals) {
            LoxValue::mark_value(pair.second, queue);
        }
        for (auto vm : vm_list) {
            for (auto & value : vm->stack()) {
                LoxValue::mark_value(value, queue);
            }
            for (auto & frame : vm->frames()) {
                LoxObject::mark(frame.closure, queue);
            }
        }
        return queue;
    }

}