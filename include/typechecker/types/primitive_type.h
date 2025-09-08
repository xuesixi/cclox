//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_PRIMITIVETYPE_H
#define CCLOX_PRIMITIVETYPE_H

#include "typechecker/lox_type.h"

/**
 * 特殊类型，包括内建的int, float, bool, nil, String，any, 以及用于解释器的unspecified, mismatched
 */
class PrimitiveType : public LoxType {
public:
    PrimitiveType(LoxTypeEnum type_enum) {
        static_assert(!std::is_abstract_v<PrimitiveType>);
        this->type_enum = type_enum;
    }

    std::string to_no_parenthesis_string() override;

    bool accept(TypePtr other) const override;

    static TypePtr IntType;
    static TypePtr FloatType;
    static TypePtr BoolType;
    static TypePtr AnyType;
    static TypePtr StringType;
    static TypePtr MismatchedType;
    static TypePtr UnspecifiedType;
    static TypePtr NilType;
};

#endif //CCLOX_PRIMITIVETYPE_H