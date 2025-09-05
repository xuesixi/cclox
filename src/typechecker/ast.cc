//
// Created by Yue Xue  on 9/5/25.
//
#include "typechecker/ast.h"

void ClassName::check_duplicate_member(const std::string &member_name) {
    auto field_found = fields.find(member_name);
    if (field_found != fields.end()) {
        throw DuplicateNameVariableError(fmt::format("redefine class member {}", member_name));
    }
    auto method_found = methods.find(member_name);
    if (method_found != methods.end()) {
        throw DuplicateNameVariableError(fmt::format("redefine class member {}", member_name));
    }
}

size_t AstAnalyzer::add_class(const std::string &name) {
    check_duplicate(name);
    ClassName new_class(name);
    auto id = new_class.class_id;
    global_classes.insert({name, std::move(new_class)});
    return id;
}

void AstAnalyzer::add_class_member(const std::string &class_name, ClassName::ClassMemberType member_type,
    const std::string &member_name, TypePtr &type) {
    auto found = global_classes.find(class_name);
    if (found == global_classes.end()) {
        implementation_error("a class name is not found");
    }
    ClassName &the_class = found->second;
    the_class.check_duplicate_member(member_name);
    if (member_type == ClassName::ClassMemberType::Field) {
        the_class.fields.insert({member_name, type});
    } else if (member_type == ClassName::ClassMemberType::Method){
        if (type->type_enum != LoxTypeEnum::Function) {
            implementation_error("the method is not a function type");
        } else {
            the_class.methods.insert({member_name, std::static_pointer_cast<FunctionType>(type)});
        }
    } else {
        implementation_error("not implemented yet");
    }
}

void AstAnalyzer::check_duplicate(const std::string &name) {
    auto cls_found = global_classes.find(name);
    if (cls_found != global_classes.end()) {
        throw DuplicateNameVariableError(fmt::format("the name {} is redefined", name));
    }
    auto var_found = global_functions.find(name);
    if (var_found != global_functions.end()) {
        throw DuplicateNameVariableError(fmt::format("the name {} is redefined", name));
    }
}

extern AstAnalyzer ast_analyzer;
