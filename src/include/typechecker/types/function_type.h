//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_FUNCTION_TYPE_H
#define CCLOX_FUNCTION_TYPE_H

#include "lox_type.h"

class FunctionType : public LoxType {
public:
    FunctionType(std::vector<TypePtr> &&p, TypePtr &&rt) : parameters(std::move(p)), return_type(std::move(rt)) {
        type_enum = LoxTypeEnum::Function;
    }

    std::vector<TypePtr> parameters;
    TypePtr return_type;

private:
    std::string to_no_parenthesis_string() override {
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
};

#endif //CCLOX_FUNCTION_TYPE_H