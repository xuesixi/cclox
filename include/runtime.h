//
// Created by Yue Xue  on 8/4/25.
//

#ifndef CCLOX_RUNTIME_H
#define CCLOX_RUNTIME_H

#include "common.h"
#include <barrier>
#include <latch>
#include <list>
#include <unordered_set>
#include "object.h"
#include "value.h"
#include "cclox_util.h"

class LoxObject;
class VM;

namespace Runtime {
    extern thread_local int thread_id;

    extern std::atomic_flag sync_flag;

    extern std::vector<std::weak_ptr<LoxObject> > weak_pool;

    extern std::unordered_map<uint16_t, Value> globals;

    extern std::unordered_map<uint16_t, Value> builtin;

    extern bool allow_gc;

    extern std::unordered_set<int> vm_to_wait;

    /**
     * 用于全局变量的访问
     * */
    extern std::mutex globals_access_mutex;

    /**
     * 储存了所有的vm。
     * */
    extern std::list<VM *> vm_list;

    extern std::optional<std::latch> pause_latch;

    extern std::optional<std::latch> resume_latch;

    extern std::atomic<bool> in_gc;

    extern std::optional<std::ofstream> log_stream;

    void print_log(const std::string &content, Color color);

    /**
     * 已被分配的空间的大小。会在分配新的LoxObject的时候被增加、在销毁时被减少。
     * 其数值并不准确，但足够用来进行是否要gc的判断。
     */
    extern std::atomic<size_t> allocated_size;

    /**
     * 检查是否要gc，如果要，则gc。
     * gc并不由额外的线程执行，而是由触发本次gc的那个vm线程执行。
     */
    void check_gc();

    void respond_gc();

    /**
     * 如果gc正在进行，则配合之。如果有其他涉及sync_flag的操作，则自旋等待之。
     * 最终得到自旋锁的控制权。
     * 该函数的调用者必须在合适的时机调用sync_flag.clear()
     */
    void wait_sync();

    inline void register_object(const std::shared_ptr<LoxObject> &ptr) {
        wait_sync();
        weak_pool.push_back(std::weak_ptr{ptr});
        sync_flag.clear();
    }

    /**
     * 标记所有根节点。包括全局变量池，以及所有现存的vm的栈空间、帧栈函数
     * @return 将被标记的对象以一个queue返回。
     */
    std::queue<LoxReference> mark_roots();

    /**
     * 估算本对象的内存占用（包括本对象的本体，但不包括其他loxobject本体），使gc的内存分配记录增加合适的值，并使其不再受到gc保护
     * 该函数应该在本对象的内存占用被固定，且gc安全之后才使用，且仅能使用一次。如果有container成员，那么应该尽可能将其capacity先缩减为size，减少冗余
     */
    template<typename T>
    void record_allocation(const std::shared_ptr<T> &reference) {
        static_assert(std::is_base_of_v<LoxObject, T>);
        auto old = allocated_size.fetch_add(reference->compute_size());
        reference->is_protected = false;
        if (Flag::show_heap) {
            print_log(fmt::format("@{} [+] heap: {:^6} -> {:^6}; {}\n", nanos_str(), old,
                                  old + reference->compute_size(), reference->to_visual_string()),
                      Color::BRIGHT_YELLOW);
        }
    }

    /**
     * 该函数应该用在LoxObject的析构函数中。它会将gc记录的分配空间记录减少彼对象的compute_size()的值
     */
    inline void record_free(LoxObject &lox_object) {
        auto old = allocated_size.fetch_sub(lox_object.compute_size());
        if (Flag::show_heap) {
            print_log(fmt::format("@{} [-] heap: {:^6} -> {:^6}; {}\n", nanos_str(), old,
                                  old - lox_object.compute_size(), lox_object.to_visual_string()),
                      Color::BRIGHT_YELLOW);
        }
    }

    /**
     * 分配一个指定类型的对象，将其注册在weak_pool中。该函数的调用者必须手动在合适的时机进行record_allocation()
     * @return 对应类型的shared_ptr
     */
    template<typename T, typename... Args>
    static std::shared_ptr<T> allocate_as(Args &&... args) {
        static_assert(std::is_base_of_v<LoxObject, T>, "The template argument has to be a subclass of LoxObject");
        check_gc();
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        register_object(ptr);
        return ptr;
    }

    /**
     * allocate_as的包装。返回一个LoxReference而非shared_ptr<T>，在某些情况下用起来更方便。该函数的调用者必须手动在合适的时机进行record_allocation()
     */
    template<typename T, typename... Args>
    static LoxReference allocate_as_ref(Args &&... args) {
        auto ptr = allocate_as<T>(std::forward<Args>(args)...);
        return std::static_pointer_cast<LoxObject>(ptr);
    }
}


#endif //CCLOX_RUNTIME_H
