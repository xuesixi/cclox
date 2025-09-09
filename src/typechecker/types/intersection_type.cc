//
// Created by Yue Xue  on 9/6/25.
//

#include "typechecker/types/intersection_type.h"
#include <sstream>

bool IntersectionType::accept(TypePtr other) const {
    // 如果是普通类型，检查是否全部符合
    if (other->type_enum != LoxTypeEnum::Intersection) {
        for (auto & type : intersections) {
            if (type->accept(other) == false) {
                return false;
            }
        }
        return true;
    }
    // 如果也是交集，则检查other是否包含了自身的所有元素
    const auto other_inter = std::static_pointer_cast<IntersectionType>(other);
    for (auto & type : intersections) {
        if (other_inter->accept(type) == false) {
            return false;
        }
    }
    return true;
}

std::string IntersectionType::to_no_parenthesis_string() {
    std::stringstream ss;
    size_t i = 0;
    for (const auto & type : intersections) {
        ss << type->to_string();
        if (i != intersections.size() - 1) {
            ss << " & ";
        }
        i ++;
    }
    return ss.str();
}
