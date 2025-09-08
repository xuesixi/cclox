//
// Created by Yue Xue  on 9/6/25.
//

#include "typechecker/types/union_type.h"

#include <sstream>

bool UnionType::accept(TypePtr other) const {
    // 如果other是一个普通类型，那么一一比对
    if (other->type_enum != LoxTypeEnum::Union) {
        for (auto & type : unions) {
            if (type->accept(other)) {
                return true;
            }
        }
        return false;
    }
    // 如果other也是一个union，则本union必须完全包括其所有元素
    auto other_union = std::static_pointer_cast<UnionType>(other);
    for (auto & type : other_union->unions) {
        if (accept(type) == false) {
            return false;
        }
    }
    return true;
}

bool UnionType::contains_nil() const {
    for (auto & type : unions) {
        if (type->type_enum == LoxTypeEnum::Nil) {
            return true;
        }
    }
    return false;
}

std::string UnionType::to_no_parenthesis_string() {
    std::stringstream ss;
    size_t i = 0;
    for (auto & type : unions) {
        ss << type->to_string();
        if (i != unions.size() - 1) {
            ss << " | ";
        }
        i ++;
    }
    return ss.str();
}
