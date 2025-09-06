//
// Created by Yue Xue  on 9/6/25.
//

#include "typechecker/types/tuple_type.h"
#include <sstream>

bool TupleType::accept(TypePtr other) const {
    if (other->type_enum != LoxTypeEnum::Tuple) {
        return false;
    }
    auto other_tuple = std::static_pointer_cast<TupleType>(other);
    if (other_tuple->tuple.size() != tuple.size()) {
        return false;
    }
    for (size_t i = 0; i < tuple.size(); i ++) {
        if (tuple.at(i)->accept(other_tuple->tuple.at(i)) == false) {
            return false;
        }
    }
    return true;
}

std::string TupleType::to_no_parenthesis_string() {
    std::stringstream ss;
    ss << "(";
    for (size_t i = 0; i < tuple.size(); ++i) {
        ss << tuple.at(i)->to_string();
        if (i != tuple.size() - 1) {
            ss << ", ";
        }
    }
    ss << ")";
    return ss.str();
}
