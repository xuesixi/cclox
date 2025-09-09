//
// Created by Yue Xue  on 9/6/25.
//

#include "typechecker/types/primitive_type.h"

std::string PrimitiveType::to_no_parenthesis_string() {
    switch (type_enum) {
        case LoxTypeEnum::Unspecified: return "unspecified";
        case LoxTypeEnum::Mismatched: return "mismatched";
        case LoxTypeEnum::Any: return "any";
        case LoxTypeEnum::Int: return "int";
        case LoxTypeEnum::Float: return "float";
        case LoxTypeEnum::Bool: return "bool";
        case LoxTypeEnum::Nil: return "nil";
        case LoxTypeEnum::String: return "String";
        case LoxTypeEnum::Void: return "void";
        default:
            ASSERT_UNREACHABLE();
    }
}

bool PrimitiveType::accept(TypePtr other) const {
    if (type_enum == LoxTypeEnum::Any) {
        return true;
    }
    return other->type_enum == type_enum;
}

TypePtr PrimitiveType::IntType = std::make_shared<PrimitiveType>(LoxTypeEnum::Int);
TypePtr PrimitiveType::FloatType = std::make_shared<PrimitiveType>(LoxTypeEnum::Float);
TypePtr PrimitiveType::BoolType = std::make_shared<PrimitiveType>(LoxTypeEnum::Bool);
TypePtr PrimitiveType::VoidType = std::make_shared<PrimitiveType>(LoxTypeEnum::Void);
TypePtr PrimitiveType::AnyType  = std::make_shared<PrimitiveType>(LoxTypeEnum::Any);
TypePtr PrimitiveType::StringType = std::make_shared<PrimitiveType>(LoxTypeEnum::String);
TypePtr PrimitiveType::MismatchedType = std::make_shared<PrimitiveType>(LoxTypeEnum::Mismatched);
TypePtr PrimitiveType::UnspecifiedType = std::make_shared<PrimitiveType>(LoxTypeEnum::Unspecified);
TypePtr PrimitiveType::NilType = std::make_shared<PrimitiveType>(LoxTypeEnum::Nil);
