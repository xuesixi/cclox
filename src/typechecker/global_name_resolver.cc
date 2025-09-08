//
// Created by Yue Xue  on 9/7/25.
//

#include "typechecker/global_name_resolver.h"
#include "typechecker/types/class_type.h"
#include "typechecker/lox_type.h"
#include "typechecker/types/primitive_type.h"
#include "typechecker/st_runtime.h"

GlobalNameResolver name_resolver;

GlobalNameResolver::ClassRecord::ClassRecord(const std::string &class_name) {
    class_type = std::make_shared<ClassType>(class_name);
    class_id = class_type->get_type_id();
}

void GlobalNameResolver::ClassRecord::check_duplicate_member(const std::string &member_name) {
    auto field_found = fields.find(member_name);
    if (field_found != fields.end()) {
        throw DuplicateNameVariableError(fmt::format("redefine class member {}", member_name));
    }
    auto method_found = methods.find(member_name);
    if (method_found != methods.end()) {
        throw DuplicateNameVariableError(fmt::format("redefine class member {}", member_name));
    }
}

size_t GlobalNameResolver::resolve_type_id(const std::string &name) {
    auto found = typename_to_id.find(name);
    if (found != typename_to_id.end()) {
        return found->second;
    }
    id_to_typename.push_back(name);
    size_t id = id_to_typename.size() - 1;
    typename_to_id.insert({name, id});
    return id;
}

std::string GlobalNameResolver::read_typename_from_id(size_t type_id) {
    return id_to_typename.at(type_id);
}

void GlobalNameResolver::declare_function(const std::string &name, TypePtr &type) {
    if (resolve_name(name).first->type_enum == LoxTypeEnum::Unspecified) {
        auto index = st_runtime.declare_global();
        global_functions.insert({name, {type, index}});
    } else {
        throw DuplicateNameVariableError(fmt::format("the name {} is already defined", name));
    }
}

void GlobalNameResolver::declare_class(const std::string &class_name) {
    if (resolve_name(class_name).first->type_enum == LoxTypeEnum::Unspecified ) {
        auto index = st_runtime.declare_global();
        global_classes.insert({class_name, {{class_name}, index}});
    } else {
        throw DuplicateNameVariableError(fmt::format("the name {} is already defined", class_name));
    }
}

void GlobalNameResolver::declare_class_member(const std::string &class_name, ClassRecord::ClassMemberType member_type,
                                  const std::string &member_name, TypePtr &type) {
    auto found = global_classes.find(class_name);
    if (found == global_classes.end()) {
        IMPL_ERROR("a class name is not found");
    }
    auto &pair = found->second;
    ClassRecord &class_record = pair.first;
    class_record.check_duplicate_member(member_name);
    if (member_type == ClassRecord::ClassMemberType::Field) {
        class_record.fields.insert({member_name, type});
    } else if (member_type == ClassRecord::ClassMemberType::Method) {
        if (type->type_enum != LoxTypeEnum::Function) {
            IMPL_ERROR("the method is not a function type");
        } else {
            class_record.methods.insert({member_name, type});
        }
    } else {
        NOT_IMPLEMENTED();
    }
}

TypePtr GlobalNameResolver::find_class_member(size_t type_id, const std::string &member_name) {
    std::string name = read_typename_from_id(type_id);
    auto found = global_classes.find(name);
    if (found == global_classes.end()) {
        throw ClassNotFoundError(fmt::format("the class {} is not defined", name));
    }
    auto &class_record = found->second.first;
    auto found_method = class_record.methods.find(member_name);
    if (found_method != class_record.methods.end()) {
        return found_method->second;
    }
    auto found_field = class_record.fields.find(member_name);
    if (found_field != class_record.fields.end()) {
        return found_field->second;
    }
    throw ClassMemberNotFoundError(fmt::format("the class {} has no such member {}", name, member_name));
}

std::pair<TypePtr, uint16_t> GlobalNameResolver::resolve_name(const std::string &name) {
    auto cls_found = global_classes.find(name);
    if (cls_found != global_classes.end()) {
        auto &pair = cls_found->second;
        return {pair.first.class_type, pair.second};
    }
    auto var_found = global_functions.find(name);
    if (var_found != global_functions.end()) {
        return var_found->second;
    }
    return {PrimitiveType::UnspecifiedType, 0};
}

// TypePtr GlobalNameResolver::find_class(const std::string &name) {
//     auto found = global_classes.find(name);
//     if (found == global_classes.end()) {
//         throw ClassNotFoundError(fmt::format("the class {} is not defined", name));
//     }
//     return found->second.class_type;
// }
//
// TypePtr GlobalNameResolver::find_function(const std::string &fun_name) {
//     auto found = global_functions.find(fun_name);
//     if (found == global_functions.end()) {
//         throw VariableNotFoundError(fmt::format("the function {} is not defined", fun_name));
//     }
//     return found->second;
// }
