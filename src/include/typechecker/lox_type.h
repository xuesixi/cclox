//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_LOX_TYPE_H
#define CCLOX_LOX_TYPE_H

#include <memory>
#include <string>
#include "error.h"
#include <sstream>
#include <unordered_set>

enum class LoxTypeEnum {
    Unspecified,
    Mismatched,
    Any,
    Int,
    Bool,
    Float,
    String,
    Union,
    Intersection,
    Tuple,
    Array,
    Function,
    Class,
};

class LoxType;
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
     * 获取一个name对应的typeid
     */
    static size_t resolve_type_id(const std::string &name);

    /**
     * 根据typeid，读取对应类型的名字
     */
    static std::string read_typename_from_id(size_t type_id);

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

    static std::unordered_map<std::string, size_t> typename_to_id;

    virtual std::string to_no_parenthesis_string() = 0;

    static std::vector<std::string> id_to_typename;
};


#endif //CCLOX_LOX_TYPE_H