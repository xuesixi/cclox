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
    Nil,
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
    class ClassRecord {
    public:
        enum class ClassMemberType {
            Field,
            Method
        };

        friend class LoxType;

        ClassRecord(const std::string &class_name) {
            class_id = LoxType::resolve_type_id(class_name);
        }

        void check_duplicate_member(const std::string &member_name) {
            auto field_found = fields.find(member_name);
            if (field_found != fields.end()) {
                throw DuplicateNameVariableError(fmt::format("redefine class member {}", member_name));
            }
            auto method_found = methods.find(member_name);
            if (method_found != methods.end()) {
                throw DuplicateNameVariableError(fmt::format("redefine class member {}", member_name));
            }
        }

    private:
        size_t class_id;
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
     * 记录一个新的函数
     */
    static void record_function(const std::string &name, TypePtr &type) {
        check_duplicate(name);
        global_functions.insert({name, type});
    }

    static void record_class(const std::string &name) {
        check_duplicate(name);
        ClassRecord new_class(name);
        global_classes.insert({name, std::move(new_class)});
    }

    static void record_class_member(const std::string &class_name, ClassRecord::ClassMemberType member_type,
                                    const std::string &member_name, TypePtr &type) {
        auto found = global_classes.find(class_name);
        if (found == global_classes.end()) {
            implementation_error("a class name is not found");
        }
        ClassRecord &the_class = found->second;
        the_class.check_duplicate_member(member_name);
        if (member_type == ClassRecord::ClassMemberType::Field) {
            the_class.fields.insert({member_name, type});
        } else if (member_type == ClassRecord::ClassMemberType::Method) {
            if (type->type_enum != LoxTypeEnum::Function) {
                implementation_error("the method is not a function type");
            } else {
                the_class.methods.insert({member_name, type});
            }
        } else {
            implementation_error("not implemented yet");
        }
    }

    static TypePtr find_class_member(size_t type_id, const std::string &member_name) {
        std::string name = LoxType::read_typename_from_id(type_id);
        auto found = global_classes.find(name);
        if (found == global_classes.end()) {
            throw ClassNotFoundError(fmt::format("the class {} is not defined", name));
        }
        auto &class_name = found->second;
        auto found_method = class_name.methods.find(member_name);
        if (found_method != class_name.methods.end()) {
            return found_method->second;
        }
        auto found_field = class_name.fields.find(member_name);
        if (found_field != class_name.fields.end()) {
            return found_field->second;
        }
        throw ClassMemberNotFoundError(fmt::format("the class {} has no such member {}", name, member_name));
    }

    static TypePtr find_function(const std::string &fun_name) {
        // todo
        // auto found
        return nullptr;
    }

    static void check_duplicate(const std::string &name) {
        auto cls_found = global_classes.find(name);
        if (cls_found != global_classes.end()) {
            throw DuplicateNameVariableError(fmt::format("the name {} is redefined", name));
        }
        auto var_found = global_functions.find(name);
        if (var_found != global_functions.end()) {
            throw DuplicateNameVariableError(fmt::format("the name {} is redefined", name));
        }
    }

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
