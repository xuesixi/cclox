//
// Created by Yue Xue  on 8/3/25.
//

#ifndef CCLOX_OBJECT_H
#define CCLOX_OBJECT_H

#include <string>
#include "runtime.h"

using LoxReference = std::shared_ptr<LoxObject>;

class LoxObject {
public:

    LoxObject() = default;

    virtual ~LoxObject() = default;

    virtual void clear_reference() = 0;

    virtual bool operator==(const LoxObject &other) const = 0;

    [[nodiscard]] virtual std::string to_string() const = 0;

    template<typename T, typename... Args>
    static LoxReference allocate(Args... args) {
        auto ptr = allocate_as<T>(std::forward<Args>(args)...);
        return std::static_pointer_cast<LoxObject>(ptr);
    }

    template<typename T, typename... Args>
    static std::shared_ptr<T> allocate_as(Args... args) {
        static_assert(std::is_base_of_v<LoxObject, T>, "The template argument has to be a subclass of LoxObject");
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        runtime.register_object(ptr);
        return ptr;
    }

};


#endif //CCLOX_OBJECT_H
