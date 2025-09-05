//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_FUNCTION_TYPE_H
#define CCLOX_FUNCTION_TYPE_H

#include "../lox_type.h"

class FunctionType : public LoxType {
public:
    friend class CallExpression;
    FunctionType(std::vector<TypePtr> &&p, TypePtr &&rt) : parameters(std::move(p)), return_type(std::move(rt)) {
        static_assert(!std::is_abstract_v<FunctionType>);
        type_enum = LoxTypeEnum::Function;
    }

    /**
     * 这个是判断函数自身的类型是否符合，而不是判断该函数参数的类型。
     * 例如说，某处要求传入一个`(int) -> bool`函数，我们不可以传入`(int) -> int`。
     */
    bool accept(TypePtr other) const override {
        if (other->type_enum != LoxTypeEnum::Function) {
            return false;
        }
        auto other_fun = std::static_pointer_cast<FunctionType>(other);

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
    std::vector<TypePtr> parameters;
    TypePtr return_type;
};

#endif //CCLOX_FUNCTION_TYPE_H