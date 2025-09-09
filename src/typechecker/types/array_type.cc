//
// Created by Yue Xue  on 9/6/25.
//

#include "typechecker/types/array_type.h"

bool ArrayType::accept(TypePtr other) const {
    if (other->type_enum != LoxTypeEnum::Array) {
        return false;
    }
    const auto other_arr = std::static_pointer_cast<ArrayType>(other);
    return element_type->accept(other_arr->element_type);
}

std::string ArrayType::to_no_parenthesis_string() {
    return fmt::format("{}[]", element_type->to_string());
}
