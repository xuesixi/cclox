//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_FUNCTION_TYPE_H
#define CCLOX_FUNCTION_TYPE_H

#include "typechecker/lox_type.h"

class FunctionType : public LoxType {
public:
    friend class CallExpression;
    FunctionType(std::vector<TypePtr> &&parem, TypePtr &&return_type) : parameters(std::move(parem)), return_type(std::move(return_type)) {
        static_assert(!std::is_abstract_v<FunctionType>);
        type_enum = LoxTypeEnum::Function;
    }

    /**
     * 这个是判断函数自身的类型是否符合，而不是判断该函数参数的类型。
     * 例如说，某处要求传入一个`(int) -> bool`函数，我们不可以传入`(int) -> int`。
     */
    bool accept(TypePtr other) const override;

private:
    std::string to_no_parenthesis_string() override;

    std::vector<TypePtr> parameters;
    TypePtr return_type;
};

#endif //CCLOX_FUNCTION_TYPE_H