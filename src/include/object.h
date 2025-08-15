//
// Created by Yue Xue  on 8/3/25.
//

#ifndef CCLOX_OBJECT_H
#define CCLOX_OBJECT_H

#include <iostream>
#include <string>

#include "common.h"
#include "runtime.h"
#include "fmt/core.h"

using LoxReference = std::shared_ptr<LoxObject>;

class LoxObject {
public:

    LoxObject() = default;

    /**
     * 由于大部份内存方面的工作都由RAII处理，本函数仅仅减少gc中的内存分配记录。该值应该和fix_size中的对应。
     */
    virtual ~LoxObject() = default;

    /**
     * 估算本对象的内存占用（包括本对象的本体，但不包括其他LoxObject本体），使gc的内存分配记录增加合适的值。
     * 该函数应该在本对象的内存占用被固定之后才使用，且仅能使用一次。如果有vector成员，那么应该尽可能将其capacity先缩减为size。
     */
    void fix_size() {
        auto old = runtime.allocated_size.fetch_add(get_size());
        if (Flag::show_heap) {
            std::cout << fmt::format("[+] heap: {:^6} -> {:^6}; {}\n", old, old + get_size(), to_string());
        }
    }

    /**
     * 在week_pool中储存的是弱指针，如果我们判断存在循环引用孤岛，因此我们需要该函数来将引用置空，这将起到打破循环的作用。
     */
    virtual void clear_reference() = 0;

    virtual bool operator==(const LoxObject &other) const = 0;

    [[nodiscard]] virtual std::string to_string() const = 0;

    /**
     * allocate_as的包装。返回一个LoxReference而非shared_ptr<T>，在某些情况下用起来更方便。该函数的调用者必须手动在合适的时机进行fix_size()
     */
    template<typename T, typename... Args>
    static LoxReference allocate_as_ref(Args... args) {
        auto ptr = allocate_as<T>(std::forward<Args>(args)...);
        return std::static_pointer_cast<LoxObject>(ptr);
    }

    /**
     * 分配一个指定类型的对象，将其注册在weak_pool中。该函数的调用者必须手动在合适的时机进行fix_size()
     * @return 对应类型的shared_ptr
     */
    template<typename T, typename... Args>
    static std::shared_ptr<T> allocate_as(Args... args) {
        static_assert(std::is_base_of_v<LoxObject, T>, "The template argument has to be a subclass of LoxObject");
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        runtime.register_object(ptr);
        return ptr;
    }

private:

    virtual size_t get_size() = 0;
};


#endif //CCLOX_OBJECT_H
