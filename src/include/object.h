//
// Created by Yue Xue  on 8/3/25.
//

#ifndef CCLOX_OBJECT_H
#define CCLOX_OBJECT_H

#include <iostream>
#include <string>

#include "common.h"
#include "fmt/core.h"

class LoxObject;

using LoxReference = std::shared_ptr<LoxObject>;

class LoxObject {
public:

    LoxObject() = default;

    /**
     * 由于大部份内存方面的工作都由RAII处理，本函数仅仅减少gc中的内存分配记录。该值应该和fix_size中的对应。
     */
    virtual ~LoxObject() = default;

    /**
     * 估算本对象的内存占用（包括本对象的本体，但不包括其他loxobject本体
     */
    virtual size_t compute_size() = 0;

    /**
     * 在weak_pool中储存的是弱指针，如果我们判断存在循环引用孤岛，因此我们需要该函数来将引用置空，这将起到打破循环的作用。
     */
    virtual void clear_reference() = 0;

    virtual bool operator==(const LoxObject &other) const = 0;

    [[nodiscard]] virtual std::string to_string() const = 0;

};


#endif //CCLOX_OBJECT_H
