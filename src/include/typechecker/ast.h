//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_AST_H
#define CCLOX_AST_H

#include <error.h>
#include <memory>
#include <string>
#include <unordered_map>
#include "fmt/core.h"
#include "lox_type.h"
#include "typechecker/types/function_type.h"

class ClassName {
public:
    enum class ClassMemberType {
        Field,
        Method
    };
    friend class AstAnalyzer;
    ClassName(const std::string &class_name) {
        class_id = LoxType::resolve_type_id(class_name);
    }

    void check_duplicate_member(const std::string &member_name);

private:
    size_t class_id;
    std::unordered_map<std::string, std::shared_ptr<FunctionType>> methods;
    std::unordered_map<std::string, TypePtr> fields;
};

class AstAnalyzer {
public:
    void add_variable(const std::string &name, TypePtr &type) {
        check_duplicate(name);
        global_functions.insert({name, type});
    }

    /**
     * 添加一个新的类，返回其类型id
     */
    size_t add_class(const std::string &name);

    /**
     * 向指定的类中添加一个新的成员。
     * */
    void add_class_member(const std::string &class_name, ClassName::ClassMemberType member_type, const std::string &member_name, TypePtr &type);

private:
    /**
     * 检查是否已经存在同名的函数或者类。如果存在重复，则抛出异常
     */
    void check_duplicate(const std::string &name);

    std::unordered_map<std::string, TypePtr> global_functions; // 函数
    std::unordered_map<std::string, ClassName> global_classes;
};

extern AstAnalyzer ast_analyzer;

#endif //CCLOX_AST_H