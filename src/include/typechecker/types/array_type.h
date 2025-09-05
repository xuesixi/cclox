//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_ARRAY_TYPE_H
#define CCLOX_ARRAY_TYPE_H
#include "lox_type.h"

class ArrayType : public LoxType {
public:
    ArrayType(TypePtr &&ptr) : element_type(std::move(ptr)) {
        static_assert(!std::is_abstract_v<ArrayType>);
        type_enum = LoxTypeEnum::Array;
    }

    TypePtr element_type;

    bool is_subtype_of(TypePtr other) const override {

    }

private:
    std::string to_no_parenthesis_string() override {
        return fmt::format("{}[]", element_type->to_string());
    }
};

#endif //CCLOX_ARRAY_TYPE_H