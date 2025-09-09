//
// Created by Yue Xue  on 9/6/25.
//

#include "typechecker/types/class_type.h"
#include "typechecker/global_name_resolver.h"

ClassType::ClassType(const std::string &class_name) {
    // todo: 同名类型冲突
    static_assert(!std::is_abstract_v<ClassType>);
    type_id = name_resolver.resolve_type_id(class_name);
    type_enum = LoxTypeEnum::Class;
}

bool ClassType::accept(TypePtr other) const {
    if (other->type_enum == LoxTypeEnum::Nil) {
        return true;
    }
    if (other->type_enum != LoxTypeEnum::Class) {
        return false;
    }
    const std::shared_ptr<ClassType> other_class = std::static_pointer_cast<ClassType>(other);
    return type_id == other_class->type_id;
}

size_t ClassType::get_type_id() const {
    return type_id;
}

std::string ClassType::to_no_parenthesis_string() {
    return name_resolver.read_typename_from_id(type_id);
}
