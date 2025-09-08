//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_LOX_TYPE_H
#define CCLOX_LOX_TYPE_H

#include "common.h"
#include "error.h"
#include "scanner.h"

enum class LoxTypeEnum {
    Unspecified,
    Mismatched,
    Any,
    Int,
    Bool,
    Float,
    String,
    Nil,
    Union,
    Intersection,
    Tuple,
    Array,
    Function,
    Class,
};

class LoxType;
class ClassType;
using TypePtr = std::shared_ptr<LoxType>;

class LoxType {
public:

    virtual ~LoxType() {
    }

    std::string to_string() {
        if (parenthesized) {
            return fmt::format("({})", to_no_parenthesis_string());
        } else {
            return to_no_parenthesis_string();
        }
    }

    /**
     * 判断本类型是否接受other作为一个适配的类型
     */
    virtual bool accept(TypePtr other) const = 0;


    /**
     * 判断两者是否否是某种类型
     */
    static bool both_of_type(const TypePtr &a, const TypePtr &b, LoxTypeEnum type_enum);

    /**
     * 判断两者是否分别是一个int和一个float
     */
    static bool int_and_float(const TypePtr &a, const TypePtr &b);

    /**
     * 是否带有括号。转化为字符串的时候会用到
     */
    bool parenthesized = false;

    LoxTypeEnum type_enum = LoxTypeEnum::Unspecified;


private:
    virtual std::string to_no_parenthesis_string() = 0;
};

#endif //CCLOX_LOX_TYPE_H
