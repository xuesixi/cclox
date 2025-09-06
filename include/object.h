//
// Created by Yue Xue  on 8/3/25.
//

#ifndef CCLOX_OBJECT_H
#define CCLOX_OBJECT_H

#include <iostream>
#include <queue>

#include "common.h"

class LoxObject;

using LoxReference = std::shared_ptr<LoxObject>;

enum class LoxObjectType {
    Class,
    Closure,
    Function,
    Instance,
    Method,
    String,
    Captured,
};

class LoxObject {
public:

    LoxObject() = default;

    /**
     * 由于大部份内存方面的工作都由RAII处理，本函数仅仅减少gc中的内存分配记录。该值应该和fix_size中的对应。
     */
    virtual ~LoxObject() = 0;

    /**
     * 估算本对象的内存占用（包括本对象的本体，但不包括其他loxobject本体
     */
    virtual size_t compute_size() = 0;

    /**
     * 当一个object从队列中被弹出后，调用该函数将直接可达的尚未标记过的引用标记并添加入队列。不递归。
     */
    virtual void mark_reference(std::queue<LoxReference> &queue) = 0;

    /**
     * 在weak_pool中储存的是弱指针，如果我们判断存在循环引用孤岛，因此我们需要该函数来将引用置空，这将起到打破循环的作用。
     */
    virtual void clear_reference() = 0;

    virtual bool operator==(const LoxObject &other) const {
        return this == &other;
    }

    /**
     * 该object的字符串表现
     */
    [[nodiscard]] virtual std::string to_string() const = 0;

    /**
     * 该object的专门用于展示打印的字符串表现。默认与to_string相同，但作为虚函数可以覆盖。
     */
    virtual std::string to_visual_string() const {
        return to_string();
    }

    /**
     * 该object的具体类型。相比起dynamic-cast，用这个会快一些。
     */
    virtual LoxObjectType get_object_type() const = 0;

    /**
     * 判断该object是否是某个具体的类型。
     */
    bool is_of_type(LoxObjectType type) const {
        return get_object_type() == type;
    }

    /**
     * 如果ref已经被标记，则什么都不做。否则，将之标记，并添加入队列中
     */
    template <typename T>
    static void mark(std::shared_ptr<T> &ref ,std::queue<LoxReference> &queue) {
        static_assert(std::is_base_of_v<LoxObject, T>);
        if (ref->is_marked) {
            return;
        }
        ref->is_marked = true;
        queue.push(ref);
    }

    bool is_marked = false; // 在gc中被标记过。会在入队列时被标记，避免重复进入队列。
    bool is_protected = true; // 有此标记者，会被避免被gc。

};

inline LoxObject::~LoxObject() = default;

#endif //CCLOX_OBJECT_H
