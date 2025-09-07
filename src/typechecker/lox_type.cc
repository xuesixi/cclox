//
// Created by Yue Xue  on 9/6/25.
//

#include "typechecker/lox_type.h"
#include "typechecker/types/class_type.h"

std::unordered_map<std::string, TypePtr> LoxType::global_functions;
std::unordered_map<std::string, LoxType::ClassRecord> LoxType::global_classes;

std::unordered_map<std::string, size_t> LoxType::typename_to_id;
std::vector<std::string> LoxType::id_to_typename;

LoxType::ClassRecord::ClassRecord(const Token &class_name) {
    class_type = std::make_shared<ClassType>(class_name);
    class_id = class_type->get_type_id();
}

void LoxType::ClassRecord::check_duplicate_member(const std::string &member_name) {
    auto field_found = fields.find(member_name);
    if (field_found != fields.end()) {
        throw DuplicateNameVariableError(fmt::format("redefine class member {}", member_name));
    }
    auto method_found = methods.find(member_name);
    if (method_found != methods.end()) {
        throw DuplicateNameVariableError(fmt::format("redefine class member {}", member_name));
    }
}

size_t LoxType::resolve_type_id(const std::string &name) {
    auto found = typename_to_id.find(name);
    if (found != typename_to_id.end()) {
        return found->second;
    }
    id_to_typename.push_back(name);
    size_t id = id_to_typename.size() - 1;
    typename_to_id.insert({name, id});
    return id;
}

std::string LoxType::read_typename_from_id(size_t type_id) {
    return id_to_typename.at(type_id);
}

void LoxType::record_function(const std::string &name, TypePtr &type) {
    check_duplicate(name);
    global_functions.insert({name, type});
}

void LoxType::record_class(const Token &token) {
    check_duplicate(token.get_lexeme());
    ClassRecord new_class(token);
    global_classes.insert({token.get_lexeme(), std::move(new_class)});
}

void LoxType::record_class_member(const std::string &class_name, ClassRecord::ClassMemberType member_type,
                                  const std::string &member_name, TypePtr &type) {
    auto found = global_classes.find(class_name);
    if (found == global_classes.end()) {
        IMPL_ERROR("a class name is not found");
    }
    ClassRecord &the_class = found->second;
    the_class.check_duplicate_member(member_name);
    if (member_type == ClassRecord::ClassMemberType::Field) {
        the_class.fields.insert({member_name, type});
    } else if (member_type == ClassRecord::ClassMemberType::Method) {
        if (type->type_enum != LoxTypeEnum::Function) {
            IMPL_ERROR("the method is not a function type");
        } else {
            the_class.methods.insert({member_name, type});
        }
    } else {
        NOT_IMPLEMENTED();
    }
}

TypePtr LoxType::find_class(const std::string &name) {
    auto found = global_classes.find(name);
    if (found == global_classes.end()) {
        throw ClassNotFoundError(fmt::format("the class {} is not defined", name));
    }
    return found->second.class_type;
}

TypePtr LoxType::find_class_member(size_t type_id, const std::string &member_name) {
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

TypePtr LoxType::find_function(const std::string &fun_name) {
    // todo
    NOT_IMPLEMENTED();
}

void LoxType::check_duplicate(const std::string &name) {
    auto cls_found = global_classes.find(name);
    if (cls_found != global_classes.end()) {
        throw DuplicateNameVariableError(fmt::format("the name {} is redefined", name));
    }
    auto var_found = global_functions.find(name);
    if (var_found != global_functions.end()) {
        throw DuplicateNameVariableError(fmt::format("the name {} is redefined", name));
    }
}
