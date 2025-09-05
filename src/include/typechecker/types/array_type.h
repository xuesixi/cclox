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

    bool accept(TypePtr other) const override {
        if (other->type_enum != LoxTypeEnum::Array) {
            return false;
        }
        auto other_arr = std::static_pointer_cast<ArrayType>(other);
        return element_type->accept(other_arr->element_type);
    }

private:
    std::string to_no_parenthesis_string() override {
        return fmt::format("{}[]", element_type->to_string());
    }
    TypePtr element_type;
};

#endif //CCLOX_ARRAY_TYPE_H