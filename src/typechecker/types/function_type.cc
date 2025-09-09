//
// Created by Yue Xue  on 9/6/25.
//

#include "typechecker/types/function_type.h"
#include <sstream>

bool FunctionType::accept(TypePtr other) const {
    if (other->type_enum != LoxTypeEnum::Function) {
        return false;
    }
    const auto other_fun = std::static_pointer_cast<FunctionType>(other);

    // other的返回值类型必须是本函数返回值类型的子类
    if (return_type->accept(other_fun->return_type) == false) {
        return false;
    }
    if (parameters.size() != other_fun->parameters.size()) {
        return false;
    }
    for (size_t i = 0; i < parameters.size(); ++i) {
        // other的每个参数都必须是本函数对应参数的父类
        // 例如说，本函数接受int，other接受int|float，这是可以的。但反之则不行
        if (other_fun->parameters.at(i)->accept(parameters.at(i)) == false) {
            return false;
        }
    }
    return true;
}

std::string FunctionType::to_no_parenthesis_string() {
    std::stringstream ss;
    ss << "(";

    for (size_t i = 0; i < parameters.size(); ++i) {
        ss << parameters.at(i)->to_string();
        if (i != parameters.size() - 1) {
            ss << ", ";
        }
    }
    ss << fmt::format(") -> {}", return_type->to_string());
    return ss.str();
}
