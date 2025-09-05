//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_PRIMITIVETYPE_H
#define CCLOX_PRIMITIVETYPE_H

#include "lox_type.h"

/**
 * 特殊类型，包括内建的int, float, bool, String，any, 以及用于解释器的unspecified, mismatched
 */
class PrimitiveType : public LoxType {
public:
    PrimitiveType(LoxTypeEnum type_enum) {
        this->type_enum = type_enum;
    }

    std::string to_no_parenthesis_string() override {
        switch (type_enum) {
            case LoxTypeEnum::Unspecified: return "unspecified";
            case LoxTypeEnum::Mismatched: return "mismatched";
            case LoxTypeEnum::Any: return "any";
            case LoxTypeEnum::Int: return "int";
            case LoxTypeEnum::Float: return "float";
            case LoxTypeEnum::Bool: return "bool";
            case LoxTypeEnum::String: return "String";
            default:
                implementation_error("Unknown primitive type");
                return "<bad type, something is wrong>";
        }
    }

    bool is_subtype_of(TypePtr other) override {
        answer_yes_to_any(other);
        return other->type_enum == type_enum;
    }

    static TypePtr IntType;
    static TypePtr FloatType;
    static TypePtr BoolType;
    static TypePtr AnyType;
    static TypePtr StringType;
    static TypePtr MismatchedType;
    static TypePtr UnspecifiedType;
};

#endif //CCLOX_PRIMITIVETYPE_H