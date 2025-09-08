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
    class ClassRecord {
    public:
        enum class ClassMemberType {
            Field,
            Method
        };

        friend class LoxType;

        ClassRecord(const Token &class_name);

        void check_duplicate_member(const std::string &member_name);

    private:
        size_t class_id;
        std::shared_ptr<ClassType> class_type;
        std::unordered_map<std::string, TypePtr> methods;
        std::unordered_map<std::string, TypePtr> fields;
    };

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

    /**
     * 记录一个新的函数。如果重复，抛出异常
     */
    static void record_function(const std::string &name, TypePtr &type);

    /**
     * 记录一个新的类。如果重复，抛出异常
     */
    static void record_class(const Token &token);

    /**
     * 记录一个新的类的成员
     */
    static void record_class_member(const std::string &class_name, ClassRecord::ClassMemberType member_type,
                                    const std::string &member_name, TypePtr &type);

    /**
     * 寻找一个类的类型，如果没找到该类，抛出异常
     */
    static TypePtr find_class(const std::string &name);

    /**
     * 寻找一个类的成员的类型。如果没找到该成员，抛出异常
     */
    static TypePtr find_class_member(size_t type_id, const std::string &member_name);

    /**
     * 寻找一个函数
     */
    static TypePtr find_function(const std::string &fun_name);

    /**
     * 寻找指定名字对应的类型，如果没找到，返回 Unspecified
     */
    static TypePtr find_name_type(const std::string &name);

private:
    static std::unordered_map<std::string, size_t> typename_to_id;

    virtual std::string to_no_parenthesis_string() = 0;

    static std::vector<std::string> id_to_typename;

    /**
     * 下面两个数据结构包含了在语句解析器建立 AST 的过程中，收集的全局函数和类的名字以及它们对应的类型。
     * 全局函数的类是可以互相调用的（而不必遵循定义的顺序），因为在第一次解析的时候就把它们的名字和类型都记录了。
     */
    static std::unordered_map<std::string, TypePtr> global_functions;
    static std::unordered_map<std::string, ClassRecord> global_classes;
};

#endif //CCLOX_LOX_TYPE_H
