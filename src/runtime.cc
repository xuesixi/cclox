//
// Created by Yue Xue  on 8/4/25.
//

#include "runtime.h"

#include <latch>

#include "cclox_util.h"
#include "typechecker/st_runtime.h"
#include "common.h"
#include "fmt/core.h"
#include "vm.h"

namespace Runtime {

    thread_local int thread_id;

    std::atomic_flag sync_flag = ATOMIC_FLAG_INIT;

    std::vector<std::weak_ptr<LoxObject>> weak_pool;

    std::atomic<size_t> allocated_size = 0;

    std::unordered_map<uint16_t, Value> globals;

    std::unordered_map<uint16_t, Value> builtin;

    std::atomic<bool> in_gc = false;

    std::optional<std::latch> pause_latch;

    std::optional<std::latch> resume_latch;

    std::mutex globals_access_mutex;

    std::optional<std::ofstream> log_stream;

    bool allow_gc = false;

    std::unordered_set<int> vm_to_wait;

    /**
     * 当一个新的vm被创建的时候，会被添加入该列表。当销毁时，从中移除
     */
    std::list<VM *> vm_list;

    std::mutex log_mutex;

    void print_log(const std::string &content, Color color) {
        // std::lock_guard guard{log_mutex};
        if (log_stream.has_value()) {
            print_to(log_stream.value(), content, color);
            log_stream.value().flush();
        } else {
            print_to(std::cout, content, color);
            std::cout.flush();
        }
    }

    bool need_gc() {
        //todo
        return allow_gc;
    }

    void check_gc() {
        if (need_gc() == false) {
            return;
        }
        wait_sync();

        int active_count = 0;
        for (const auto vm : vm_list) {
            if (vm->is_active()) {
                active_count ++;
            }
        }

        if (Flag::show_heap) {
            print_log(fmt::format("@{} vm {} start a gc with allocated size : {}. vm counts: {}, active_count: {}, Waiting for response: {}\n",
                nanos_str(), thread_id, allocated_size.load(), vm_list.size(), active_count, active_count-1), Color::CYAN);
            for (const auto vm : vm_list) {
                if (vm->is_active() && vm->get_vm_id() != thread_id) {
                    vm_to_wait.insert(vm->get_vm_id());
                    print_log(fmt::format("gc is waiting for vm {}\n", vm->get_vm_id()), Color::CYAN);
                }
            }
        }

        in_gc = true;

        pause_latch.emplace(active_count);
        resume_latch.emplace(active_count);

        // 在此等待，直到所有线程都响应
        pause_latch->arrive_and_wait();

        // 接下来这段区间内，只有gc的vm线程在运行，其他线程都在等待
        if (Flag::show_heap) {
            print_log(fmt::format("@{} all other threads have been paused for gc\n", nanos_str()), Color::CYAN);
        }

        // 标记根节点
        std::queue<LoxReference> queue = mark_roots();

        // 追踪标记
        while (!queue.empty()) {
            const auto ref = queue.front();
            queue.pop();
            ref->mark_reference(queue);
        }

        // 清除
        for (auto & weak : weak_pool) {
            const auto ptr = weak.lock();
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

        if (Flag::show_heap) {
            // print_log(fmt::format("@{} resume other threads from gc\n", nanos_str()), Color::RED);
            print_log(fmt::format("@{} gc ends with allocated size : {}. Resume other threads from gc\n", nanos_str(), allocated_size.load()), Color::CYAN);
        }

        in_gc = false;
        sync_flag.clear();
        vm_to_wait.clear();

        // 唤醒所有线程
        resume_latch->arrive_and_wait();

    }

    void respond_gc() {
        if (!in_gc) {
            return;
        }
        if (Flag::show_heap) {
            if (vm_to_wait.contains(thread_id) == false) {
                print_log(fmt::format("@{} --- error! thread {} has been paused for gc, but is not expected!\n", nanos_str(), thread_id), Color::RED);
            } else {
                print_log(fmt::format("@{} --- thread {} has been paused for gc\n", nanos_str(), thread_id), Color::BRIGHT_BLUE);
            }
        }
        // 告诉gc线程，本线程已经停下了。
        Runtime::pause_latch->arrive_and_wait();

        // 等待gc线程的完成
        Runtime::resume_latch->arrive_and_wait();

        if (Flag::show_heap) {
            print_log(fmt::format("@{} --- thread {} has resumed from gc\n", nanos_str(), thread_id), Color::BRIGHT_BLUE);
        }
    }

    void wait_sync() {
        while (sync_flag.test_and_set(std::memory_order_acquire)) {
            // 如果tas为true，有两种可能：正在gc，或者其他非gc操作
            // 如果是gc操作，那么respond-gc可以负责之。
            // 如果是非gc操作，那么一般比较短暂。respond_gc()会直接返回，等价于自旋等待
            respond_gc();
        }
    }

    std::queue<LoxReference> mark_roots() {
        std::queue<LoxReference> queue;
        for (auto & pair : globals) {
            LoxValue::mark_value(pair.second, queue);
        }
        for (auto & global : st_runtime.get_globals()) {
            LoxValue::mark_value(global, queue);
        }
        for (const auto vm : vm_list) {
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